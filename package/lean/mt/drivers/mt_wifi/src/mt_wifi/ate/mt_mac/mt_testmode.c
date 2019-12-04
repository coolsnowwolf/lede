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
	mt_testmode.c

*/

#ifdef COMPOS_TESTMODE_WIN
#include "config.h"
#else
#include "rt_config.h"
#endif

#define MCAST_WCID_TO_REMOVE 0  /* Pat: TODO */

struct _ATE_DATA_RATE_MAP cck_mode_mcs_to_data_rate_map[] = {
	{0, 1000},
	{1, 2000},
	{2, 5500},
	{3, 11000},
	{9, 2000},
	{10, 5500},
	{11, 11000},
};

struct _ATE_DATA_RATE_MAP ofdm_mode_mcs_to_data_rate_map[] = {
	{0, 6000},
	{1, 9000},
	{2, 12000},
	{3, 18000},
	{4, 24000},
	{5, 36000},
	{6, 48500},
	{7, 54000},
};

struct _ATE_DATA_RATE_MAP n_mode_mcs_to_data_rate_map[] = {
	{0, 6500},
	{1, 13000},
	{2, 19500},
	{3, 26000},
	{4, 39000},
	{5, 52000},
	{6, 58500},
	{7, 65000},
	{32, 6000},  /* MCS32 */
};

struct _ATE_DATA_RATE_MAP ac_mode_mcs_to_data_rate_map_bw20[] = {
	{0, 65},    /* in unit of 100k */
	{1, 130},
	{2, 195},
	{3, 260},
	{4, 390},
	{5, 520},
	{6, 585},
	{7, 650},
	{8, 780},
};

struct _ATE_DATA_RATE_MAP ac_mode_mcs_to_data_rate_map_bw40[] = {
	{0, 135},   /* in unit of 100k */
	{1, 270},
	{2, 405},
	{3, 540},
	{4, 810},
	{5, 1080},
	{6, 1215},
	{7, 1350},
	{8, 1620},
	{9, 1800},
};


struct _ATE_DATA_RATE_MAP ac_mode_mcs_to_data_rate_map_bw80[] = {
	{0, 293},   /* in unit of 100k */
	{1, 585},
	{2, 878},
	{3, 1170},
	{4, 1755},
	{5, 2340},
	{6, 2633},
	{7, 2925},
	{8, 3510},
	{9, 3900},
};

struct _ATE_DATA_RATE_MAP ac_mode_mcs_to_data_rate_map_bw160[] = {
	{0, 585},   /* in unit of 100k */
	{1, 1170},
	{2, 1755},
	{3, 2340},
	{4, 3510},
	{5, 4680},
	{6, 5265},
	{7, 5850},
	{8, 7020},
	{9, 7800},
};

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
#define ATE_ANT_USER_SEL 0x80000000
/* BIT[0]:all, BIT[X==1]:ant(X-1) sel */
struct _ATE_ANT_MAP ant_to_spe_idx_map[] = {
	/* All */
	{0x0, 0},
	{0xf, 0},
	/* 1 Ant */
	{0x1, 0},	/* Tx0 */
	{0x2, 1},	/* Tx1 */
	{0x4, 3},	/* Tx2 */
	{0x8, 9},	/* Tx3 */
	/* 2 Ant */
	{0x3, 0},
	{0x5, 2},
	{0x9, 8},
	{0x6, 4},
	{0xa, 6},
	{0xc, 16},
	/* 3 Ant */
	{0x7, 0},	/* 0_1_2 */
	{0xb, 10},	/* 0_1_3 */
	{0xd, 12},	/* 0_2_3 */
	{0xe, 18},	/* 1_2_3 */
};

struct _ATE_TXPWR_GROUP_MAP txpwr_group_map[] = {
	{2407, 2484, {DMAC_TX0_G_BAND_TARGET_PWR, DMAC_TX1_G_BAND_TARGET_PWR, DMAC_TX2_G_BAND_TARGET_PWR, DMAC_TX3_G_BAND_TARGET_PWR} },
	{4910, 5140, {DMAC_GRP0_TX0_A_BAND_TARGET_PWR, DMAC_GRP0_TX1_A_BAND_TARGET_PWR,	DMAC_GRP0_TX2_A_BAND_TARGET_PWR, DMAC_GRP0_TX3_A_BAND_TARGET_PWR} },
	{5140, 5250, {DMAC_GRP1_TX0_A_BAND_TARGET_PWR, DMAC_GRP1_TX1_A_BAND_TARGET_PWR,	DMAC_GRP1_TX2_A_BAND_TARGET_PWR, DMAC_GRP1_TX3_A_BAND_TARGET_PWR} },
	{5250, 5360, {DMAC_GRP2_TX0_A_BAND_TARGET_PWR, DMAC_GRP2_TX1_A_BAND_TARGET_PWR, DMAC_GRP2_TX2_A_BAND_TARGET_PWR, DMAC_GRP2_TX3_A_BAND_TARGET_PWR} },
	{5360, 5470, {DMAC_GRP3_TX0_A_BAND_TARGET_PWR, DMAC_GRP3_TX1_A_BAND_TARGET_PWR,	DMAC_GRP3_TX2_A_BAND_TARGET_PWR, DMAC_GRP3_TX3_A_BAND_TARGET_PWR} },
	{5470, 5580, {DMAC_GRP4_TX0_A_BAND_TARGET_PWR, DMAC_GRP4_TX1_A_BAND_TARGET_PWR,	DMAC_GRP4_TX2_A_BAND_TARGET_PWR, DMAC_GRP4_TX3_A_BAND_TARGET_PWR} },
	{5580, 5690, {DMAC_GRP5_TX0_A_BAND_TARGET_PWR, DMAC_GRP5_TX1_A_BAND_TARGET_PWR,	DMAC_GRP5_TX2_A_BAND_TARGET_PWR, DMAC_GRP5_TX3_A_BAND_TARGET_PWR} },
	{5690, 5800, {DMAC_GRP6_TX0_A_BAND_TARGET_PWR, DMAC_GRP6_TX1_A_BAND_TARGET_PWR,	DMAC_GRP6_TX2_A_BAND_TARGET_PWR, DMAC_GRP6_TX3_A_BAND_TARGET_PWR} },
	{5800, 5925, {DMAC_GRP7_TX0_A_BAND_TARGET_PWR, DMAC_GRP7_TX1_A_BAND_TARGET_PWR,	DMAC_GRP7_TX2_A_BAND_TARGET_PWR, DMAC_GRP7_TX3_A_BAND_TARGET_PWR} } };

#elif defined(MT7637)
/* todo: efuse structure need unify, MT7636 will fail in this flow */
#define EFUSE_ADDR_TX0POWER_54M_2_4G                     0x58/* MT7637 */
#define EFUSE_ADDR_TX0POWER_54M_4920_5140                0x64/* MT7637 */
#define EFUSE_ADDR_TX0POWER_54M_5150_5250                0x69/* MT7637 */
#define EFUSE_ADDR_TX0POWER_54M_5250_5360                0x6E/* MT7637 */
#define EFUSE_ADDR_TX0POWER_54M_5360_5470                0x73/* MT7637 */
#define EFUSE_ADDR_TX0POWER_54M_5470_5580                0x78/* MT7637 */
#define EFUSE_ADDR_TX0POWER_54M_5580_5690                0x7D/* MT7637 */
#define EFUSE_ADDR_TX0POWER_54M_5690_5800                0x82/* MT7637 */
#define EFUSE_ADDR_TX0POWER_54M_5810_5925                0x87/* MT7637 */

struct _ATE_TXPWR_GROUP_MAP txpwr_group_map[] = {
	{2407, 2484, {EFUSE_ADDR_TX0POWER_54M_2_4G} },
	{4910, 5140, {EFUSE_ADDR_TX0POWER_54M_4920_5140} },
	{5140, 5250, {EFUSE_ADDR_TX0POWER_54M_5150_5250} },
	{5250, 5360, {EFUSE_ADDR_TX0POWER_54M_5250_5360} },
	{5360, 5470, {EFUSE_ADDR_TX0POWER_54M_5360_5470} },
	{5470, 5580, {EFUSE_ADDR_TX0POWER_54M_5470_5580} },
	{5580, 5690, {EFUSE_ADDR_TX0POWER_54M_5580_5690} },
	{5690, 5800, {EFUSE_ADDR_TX0POWER_54M_5690_5800} },
	{5800, 5925, {EFUSE_ADDR_TX0POWER_54M_5810_5925} },
};
#else
/* todo: efuse structure need unify, MT7636 will fail in this flow */
struct _ATE_TXPWR_GROUP_MAP txpwr_group_map[] = {
	{0},
};
#endif

#ifdef MT7615
#define RSSIOFFSET_24G_WF0 0x130
#define RSSIOFFSET_24G_WF1 (RSSIOFFSET_24G_WF0 + 0x1)
#define RSSIOFFSET_24G_WF2 (RSSIOFFSET_24G_WF0 + 0x2)
#define RSSIOFFSET_24G_WF3 (RSSIOFFSET_24G_WF0 + 0x3)
#define RSSIOFFSET_5G_WF0 0x134
#define RSSIOFFSET_5G_WF1 (RSSIOFFSET_5G_WF0 + 0x1)
#define RSSIOFFSET_5G_WF2 (RSSIOFFSET_5G_WF0 + 0x2)
#define RSSIOFFSET_5G_WF3 (RSSIOFFSET_5G_WF0 + 0x3)

static UINT32 rssi_eeprom_band_24G_offset[] = {RSSIOFFSET_24G_WF0,
						RSSIOFFSET_24G_WF1,
						RSSIOFFSET_24G_WF2,
						RSSIOFFSET_24G_WF3};

static UINT32 rssi_eeprom_band_5G_offset[] = {RSSIOFFSET_5G_WF0,
						RSSIOFFSET_5G_WF1,
						RSSIOFFSET_5G_WF2,
						RSSIOFFSET_5G_WF3};

static UINT32 *_rssi_eeprom_band_offset[] = {rssi_eeprom_band_24G_offset, rssi_eeprom_band_5G_offset};

static UINT32 _n_band_offset[] = {ARRAY_SIZE(rssi_eeprom_band_24G_offset), ARRAY_SIZE(rssi_eeprom_band_5G_offset)};
#else
static UINT32 *_rssi_eeprom_band_offset[] = {NULL};
static UINT32 _n_band_offset[] = {0};
#endif

#ifdef PRE_CAL_TRX_SET2_SUPPORT
static UINT16 PreCalGroupList[] = {
	0x00ED, /* 0 - Ch group 0,2,3,5,6,7 */
	0x01FF, /*All group 0 ~ 8*/
};
static UINT8 PreCalItemList[] = {
	0x1F, /* 0 - Pre-cal Bit[0]:TXLPF, Bit[1]:TXIQ, Bit[2]:TXDC, Bit[3]:RXFI, Bit[4]:RXFD */
};
#endif /* PRE_CAL_TRX_SET2_SUPPORT */

struct rssi_offset_eeprom eeprom_rssi_offset = {
#ifndef COMPOS_TESTMODE_WIN
	.rssi_eeprom_band_offset = _rssi_eeprom_band_offset,
	.n_band_offset = _n_band_offset,
	.n_band = ARRAY_SIZE(_rssi_eeprom_band_offset),
#else
	_rssi_eeprom_band_offset,
	_n_band_offset,
	sizeof(_rssi_eeprom_band_offset) / sizeof(_rssi_eeprom_band_offset[0]),
#endif
};

#if defined(TXBF_SUPPORT) && defined(MT_MAC)
UINT8 g_EBF_certification;
UINT8 BF_ON_certification;
extern UCHAR TemplateFrame[32];
#endif /* TXBF_SUPPORT && MT_MAC */

VOID MtATEDummyFunc(VOID)
{
}
/* Private Function Prototype */
static INT32 MT_ATEMPSRelease(RTMP_ADAPTER *pAd);
static INT32 MT_ATEMPSInit(RTMP_ADAPTER *pAd);
static INT32 MT_ATEMPSLoadSetting(RTMP_ADAPTER *pAd, UINT32 band_idx);
#ifdef ATE_TXTHREAD
static INT32 MT_ATEMPSRunStatCheck(RTMP_ADAPTER *pAd, UINT32 band_idx);
#endif
#ifdef LOGDUMP_TO_FILE
static INT32 MT_ATERDDParseResult(struct _ATE_LOG_DUMP_ENTRY entry, INT idx, RTMP_OS_FD_EXT fd);
static INT MT_ATEWriteFd(RTMP_STRING *log, RTMP_OS_FD_EXT srcf);
#else
static INT32 MT_ATERDDParseResult(struct _ATE_LOG_DUMP_ENTRY entry, INT idx);
#endif
static INT32 MT_MPSTxStop(RTMP_ADAPTER *pAd);
static INT32 MT_ATELogOnOff(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 on_off, UINT32 size);
static INT32 MT_ATESetICapStart(RTMP_ADAPTER *pAd, BOOLEAN Trigger, BOOLEAN RingCapEn, UINT32 Event, UINT32 Node, UINT32 Len, UINT32 StopCycle,	UINT32 BW, UINT32 MACTriggerEvent, UINT32 SourceAddrLSB, UINT32 SourceAddrMSB, UINT32 Band);
static INT32 MT_ATEGetICapStatus(RTMP_ADAPTER *pAd);
static INT32 MT_ATEGetICapIQData(RTMP_ADAPTER *pAd, PINT32 pData, PINT32 pDataLen, UINT32 IQ_Type, UINT32 WF_Num);
/* static INT32 MT_ATEInsertLog(RTMP_ADAPTER *pAd, UCHAR *log, UINT32 log_type, UINT32 len); */

/* #if CFG_eBF_Sportan_Certification */
INT32 MT_ATEGenPkt(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 band_idx);
/* #else */
/* static INT32 MT_ATEGenPkt(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 band_idx); */
/* #endif */


INT MtATESetMacTxRx(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx)
{
	INT ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_ATE_TEST_MODE_T param;
	UINT8 testmode_en = 1;

	os_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = testmode_en;
	param.ucAteIdx = EXT_ATE_SET_TRX;
	param.Data.rAteSetTrx.ucType = TxRx;
	param.Data.rAteSetTrx.ucEnable = Enable;
	param.Data.rAteSetTrx.ucBand = BandIdx; /* TODO::Fix it after 7615 merge */
	/* Make sure FW command configuration completed for store TX packet in PLE first
	 * Use aucReserved[1] for uxATEIdx extension feasibility
	 */
	param.aucReserved[1] = INIT_CMD_SET_AND_WAIT_RETRY_RSP;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: TxRx:%x, Enable:%x, BandIdx:%x\n",
		__func__, param.Data.rAteSetTrx.ucType,
		param.Data.rAteSetTrx.ucEnable,
		param.Data.rAteSetTrx.ucBand));
	ret = MtCmdATETest(pAd, &param);
#else
	ret = MtAsicSetMacTxRx(pAd, TxRx, Enable, BandIdx);
#endif
	return ret;
}


INT MtATESetTxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx)
{
	INT ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_ATE_TEST_MODE_T param;
	UINT8 testmode_en = 1;

	os_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = testmode_en;
	param.ucAteIdx = EXT_ATE_SET_TX_STREAM;
	param.Data.rAteSetTxStream.ucStreamNum = StreamNums;
	param.Data.rAteSetTxStream.ucBand = BandIdx;
	ret =  MtCmdATETest(pAd, &param);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: StreamNum:%x BandIdx:%x\n", __func__, StreamNums, BandIdx));
#else
	ret = MtAsicSetTxStream(pAd, StreamNums, BandIdx);
#endif
	return ret;
}


INT MtATESetRxPath(RTMP_ADAPTER *pAd, UINT32 RxPathSel, UCHAR u1BandIdx)
{
	INT ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_ATE_TEST_MODE_T param;
	UINT8 testmode_en = 1;

	os_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = testmode_en;
	param.ucAteIdx = EXT_ATE_SET_RX_PATH;

	/* Set Rx Ant 2/3 for Band 1 */
	if (u1BandIdx)
		RxPathSel = RxPathSel << 2;

	param.Data.rAteSetRxPath.ucType = RxPathSel;
	param.Data.rAteSetRxPath.ucBand = u1BandIdx;
	ret = MtCmdATETest(pAd, &param);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: RxPathSel:%x BandIdx:%x\n", __func__, RxPathSel, u1BandIdx));
#else
	ret = MtAsicSetRxPath(pAd, RxPathSel, u1BandIdx);
#endif
	return ret;
}


INT MtATESetRxFilter(RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T filter)
{
	INT ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_ATE_TEST_MODE_T param;
	UINT8 testmode_en = 1;

	os_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = testmode_en;
	param.ucAteIdx = EXT_ATE_SET_RX_FILTER;
	param.Data.rAteSetRxFilter.ucBand = filter.u1BandIdx; /* TODO::Fix it after 7615 merge */

	if (filter.bPromiscuous)
		param.Data.rAteSetRxFilter.ucPromiscuousMode = 1;
	else {
		param.Data.rAteSetRxFilter.ucReportEn = (UCHAR)filter.bFrameReport;
		param.Data.rAteSetRxFilter.u4FilterMask = cpu2le32(filter.filterMask);
	}

	ret =  MtCmdATETest(pAd, &param);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: BandIdx:%x\n", __func__, filter.u1BandIdx));
#else
	ret = MtAsicSetRxFilter(pAd, filter);
#endif
	return ret;
}


INT MtATESetCleanPerStaTxQueue(RTMP_ADAPTER *pAd, BOOLEAN sta_pause_enable)
{
	INT ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _EXT_CMD_ATE_TEST_MODE_T param;
	UINT8 testmode_en = 1;

	os_zero_mem(&param, sizeof(param));
	param.ucAteTestModeEn = testmode_en;
	param.ucAteIdx = EXT_ATE_SET_CLEAN_PERSTA_TXQUEUE;
	param.Data.rAteSetCleanPerStaTxQueue.fgStaPauseEnable = sta_pause_enable;
	/* Give a same STA ID */
	param.Data.rAteSetCleanPerStaTxQueue.ucStaID = 0;

	ret =  MtCmdATETest(pAd, &param);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: sta_pause_enable:%x\n", __func__, sta_pause_enable));
#else
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: Function do not support yet.\n", __func__));
#endif
	return ret;
}

#ifdef ARBITRARY_CCK_OFDM_TX
VOID MtATEInitCCK_OFDM_Path(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 i = 0;
	UINT32 value = 0;
	UINT32 MAC_RDVLE, MAC_ADDR, MAC_WRMASK, MAC_WRVALUE;

	if (IS_ATE_DBDC(pAd)) {
		UINT32 idx = 0;

		/* PTA mux */
		if (BandIdx == 0) {
			/* 0x810600D0[4:0] = 0x8; */
			MAC_ADDR = ANT_SWITCH_CON3;
			MAC_WRMASK = 0xFFFFFFE0;
			MAC_WRVALUE = 0x8 << 0;
			MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
			/* 0x810600D4[20:16] = 0xE; */
			MAC_ADDR = ANT_SWITCH_CON4;
			MAC_WRMASK = 0xFFE0FFFF;
			MAC_WRVALUE = 0xE << 16;
			MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		} else {
			/* 0x810600E0[11:8] = 0x5; */
			MAC_ADDR = ANT_SWITCH_CON7;
			MAC_WRMASK = 0xFFFFF0FF;
			MAC_WRVALUE = 0x5 << 8;
			MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
			/* 0x810600E4[27:24] = 0xB; */
			MAC_ADDR = ANT_SWITCH_CON8;
			MAC_WRMASK = 0xF0FFFFFF;
			MAC_WRVALUE = 0xB << 24;
			MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		}

		/* iPA */
		for (i = 0; i < 2; i++) {
			idx = BandIdx * 2 + i;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s: Reset WF_%d\n", __func__, idx));
			ATEOp->RfRegRead(pAd, idx, 0x48, &value);
			value &= ~(0x3FF << 20); /* bit[29:20] */
			value |= (3 << 20);
			ATEOp->RfRegWrite(pAd, idx, 0x48, value);
		}
	} else {
		/* PTA mux */
		/* Gband */
		/* 0x810600D0[4:0] = 0x8; */
		MAC_ADDR = ANT_SWITCH_CON3;
		MAC_WRMASK = 0xFFFFFFE0;
		MAC_WRVALUE = 0x8 << 0;
		MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		/* 0x810600D4[20:16] = 0xE; */
		MAC_ADDR = ANT_SWITCH_CON4;
		MAC_WRMASK = 0xFFE0FFFF;
		MAC_WRVALUE = 0xE << 16;
		MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		/* 0x810600DC[3:0] = 0x0; */
		MAC_ADDR = ANT_SWITCH_CON6;
		MAC_WRMASK = 0xFFFFFFF0;
		MAC_WRVALUE = 0x0 << 0;
		MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		/* 0x810600E0[19:16] = 0x6; */
		MAC_ADDR = ANT_SWITCH_CON7;
		MAC_WRMASK = 0xFFF0FFFF;
		MAC_WRVALUE = 0x6 << 16;
		MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		/* Aband */
		/* 0x810600D4[12:8] = 0xD; */
		MAC_ADDR = ANT_SWITCH_CON4;
		MAC_WRMASK = 0xFFFFE0FF;
		MAC_WRVALUE = 0xD << 8;
		MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		/* 0x810600CC[28:24] = 0x13; */
		MAC_ADDR = ANT_SWITCH_CON2;
		MAC_WRMASK = 0xE0FFFFFF;
		MAC_WRVALUE = 0x13 << 24;
		MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		/* 0x810600E0[11:8] = 0x5; */
		MAC_ADDR = ANT_SWITCH_CON7;
		MAC_WRMASK = 0xFFFFF0FF;
		MAC_WRVALUE = 0x5 << 8;
		MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		/* 0x810600E4[27:24] = 0xB; */
		MAC_ADDR = ANT_SWITCH_CON8;
		MAC_WRMASK = 0xF0FFFFFF;
		MAC_WRVALUE = 0xB << 24;
		MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
		MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
		MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);

		for (i = 0; i < 4; i++) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s: Reset WF_%d\n", __func__, i));
			ATEOp->RfRegRead(pAd, i, 0x48, &value);
			value &= ~(0x3FF << 20); /* bit[29:20] */
			value |= (3 << 20);
			ATEOp->RfRegWrite(pAd, i, 0x48, value);
		}
	}
}


VOID MtATESetCCK_OFDM_Path(RTMP_ADAPTER *pAd, UINT32 TxPathSel, UCHAR BandIdx)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 i = 0;
	UINT32 value = 0;
	UINT32 MAC_RDVLE, MAC_ADDR, MAC_WRMASK, MAC_WRVALUE;

	if (IS_ATE_DBDC(pAd)) {
		UINT32 idx = 0;

		/* PTA mux */
		if (BandIdx == 0) {
			if ((TxPathSel & (1 << 0)) == 0) {
				/* 0x810600D0[4:0] = 0x1B; */
				MAC_ADDR = ANT_SWITCH_CON3;
				MAC_WRMASK = 0xFFFFFFE0;
				MAC_WRVALUE = 0x1B << 0;
				MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
				MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
				MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
			}

			if ((TxPathSel & (1 << 1)) == 0) {
				/* 0x810600D4[20:16] = 0x1B; */
				MAC_ADDR = ANT_SWITCH_CON4;
				MAC_WRMASK = 0xFFE0FFFF;
				MAC_WRVALUE = 0x1B << 16;
				MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
				MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
				MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
			}
		} else {
			if ((TxPathSel & (1 << 0)) == 0) {
				/* 0x810600E0[11:8] = 0xF; */
				MAC_ADDR = ANT_SWITCH_CON7;
				MAC_WRMASK = 0xFFFFF0FF;
				MAC_WRVALUE = 0xF << 8;
				MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
				MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
				MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
			}

			if ((TxPathSel & (1 << 1)) == 0) {
				/* 0x810600E4[27:24] = 0xF; */
				MAC_ADDR = ANT_SWITCH_CON8;
				MAC_WRMASK = 0xF0FFFFFF;
				MAC_WRVALUE = 0xF << 24;
				MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
				MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
				MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
			}
		}

		for (i = 0; i < 2; i++) {
			if ((TxPathSel & (1 << i)) == 0) {
				idx = BandIdx * 2 + i;
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s: Disable WF_%d, TxSel=%x\n", __func__, idx, TxPathSel));
				ATEOp->RfRegRead(pAd, idx, 0x48, &value);
				value &= ~(0x3FF << 20); /* bit[29:20] */
				value = value | (2 << 28) | (2 << 26) | (8 << 20);
				ATEOp->RfRegWrite(pAd, idx, 0x48, value);
			}
		}
	} else {
		/* PTA mux */
		/* Gband */
		if ((TxPathSel & (1 << 0)) == 0) {
			/* 0x810600D0[4:0] = 0x1B; */
			MAC_ADDR = ANT_SWITCH_CON3;
			MAC_WRMASK = 0xFFFFFFE0;
			MAC_WRVALUE = 0x1B << 0;
			MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		}

		if ((TxPathSel & (1 << 1)) == 0) {
			/* 0x810600D4[20:16] = 0x1B; */
			MAC_ADDR = ANT_SWITCH_CON4;
			MAC_WRMASK = 0xFFE0FFFF;
			MAC_WRVALUE = 0x1B << 16;
			MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		}

		if ((TxPathSel & (1 << 2)) == 0) {
			/* 0x810600DC[3:0] = 0xF; */
			MAC_ADDR = ANT_SWITCH_CON6;
			MAC_WRMASK = 0xFFFFFFF0;
			MAC_WRVALUE = 0xF << 0;
			MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		}

		if ((TxPathSel & (1 << 3)) == 0) {
			/* 0x810600E0[19:16] = 0xF; */
			MAC_ADDR = ANT_SWITCH_CON7;
			MAC_WRMASK = 0xFFF0FFFF;
			MAC_WRVALUE = 0xF << 16;
			MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		}

		/* Aband */
		if ((TxPathSel & (1 << 0)) == 0) {
			/* 0x810600D4[12:8] = 0x1B; */
			MAC_ADDR = ANT_SWITCH_CON4;
			MAC_WRMASK = 0xFFFFE0FF;
			MAC_WRVALUE = 0x1B << 8;
			MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		}

		if ((TxPathSel & (1 << 1)) == 0) {
			/* 0x810600CC[28:24] = 0x1B; */
			MAC_ADDR = ANT_SWITCH_CON2;
			MAC_WRMASK = 0xE0FFFFFF;
			MAC_WRVALUE = 0x1B << 24;
			MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		}

		if ((TxPathSel & (1 << 2)) == 0) {
			/* 0x810600E0[11:8] = 0xF; */
			MAC_ADDR = ANT_SWITCH_CON7;
			MAC_WRMASK = 0xFFFFF0FF;
			MAC_WRVALUE = 0xF << 8;
			MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		}

		if ((TxPathSel & (1 << 3)) == 0) {
			/* 0x810600E4[27:24] = 0xF; */
			MAC_ADDR = ANT_SWITCH_CON8;
			MAC_WRMASK = 0xF0FFFFFF;
			MAC_WRVALUE = 0xF << 24;
			MCU_IO_READ32(pAd, MAC_ADDR, &MAC_RDVLE);
			MAC_RDVLE = (MAC_RDVLE & MAC_WRMASK) | MAC_WRVALUE;
			MCU_IO_WRITE32(pAd, MAC_ADDR, MAC_RDVLE);
		}

		for (i = 0; i < 4; i++) {
			if ((TxPathSel & (1 << i)) == 0) {
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s: Disable WF_%d, TxSel=%x\n", __func__, i, TxPathSel));
				ATEOp->RfRegRead(pAd, i, 0x48, &value);
				value &= ~(0x3FF << 20); /* bit[29:20] */
				value = value | (2 << 28) | (2 << 26) | (8 << 20);
				ATEOp->RfRegWrite(pAd, i, 0x48, value);
			}
		}
	}
}
#endif


#ifdef ATE_TXTHREAD
static INT TestMode_TxThread(ULONG Context);
static INT32 TESTMODE_TXTHREAD_INIT(RTMP_ADAPTER *pAd, INT thread_idx);
static INT32 TESTMODE_TXTHREAD_RELEASE(RTMP_ADAPTER *pAd, INT thread_idx);
static VOID TESTMODEThreadProceedTx(RTMP_ADAPTER *pAd, UINT32 band_idx);
static VOID TESTMODEThreadStopTx(RTMP_ADAPTER *pAd, INT thread_idx);


static INT MT_ATETxHandler(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ate_ctrl = &pAd->ATECtrl;
	UINT32 mode = TESTMODE_GET_PARAM(ate_ctrl, band_idx, Mode);
	USHORT q_idx = TESTMODE_GET_PARAM(ate_ctrl, band_idx, QID);
	INT32 dequeue_size = ate_ctrl->deq_cnt;
	INT32 multi_users = 0;
	UINT32 txed_cnt = 0;
	UINT32 tx_cnt = 0;
	UCHAR hwq_idx = q_idx;
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;
#if defined(MT7615) || defined(MT7622)
	struct _ATE_IPG_PARAM *ipg_param = TESTMODE_GET_PADDR(ate_ctrl, band_idx, ipg_param);
	struct _ATE_TX_TIME_PARAM *tx_time_param = TESTMODE_GET_PADDR(ate_ctrl, band_idx, tx_time_param);
	UINT32 pkt_tx_time = tx_time_param->pkt_tx_time;
	UINT8 need_ampdu = tx_time_param->pkt_need_ampdu;
	UINT32 ipg = ipg_param->ipg;
#endif /* defined(MT7615) || defined(MT7622) */

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
	if (IS_MT7615(pAd) || IS_MT7622(pAd) ||	IS_P18(pAd) || IS_MT7663(pAd)) {
		UCHAR wdev_idx = TESTMODE_GET_PARAM(ate_ctrl, band_idx, wdev_idx);
		struct wifi_dev *wdev = pAd->wdev_list[wdev_idx];

		hwq_idx = arch_ops->get_resource_idx(pAd, wdev, TX_MGMT, q_idx);
	}
#endif /* defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) */
	txed_cnt = TESTMODE_GET_PARAM(ate_ctrl, band_idx, TxedCount);
	tx_cnt = TESTMODE_GET_PARAM(ate_ctrl, band_idx,	TxCount);

	do {
		ULONG free_num;
ate_thread_dequeue:
#ifdef COMPOS_TESTMODE_WIN
		free_num = 1;
#else
		free_num = 0;

		if ((q_idx & MGMT_USE_QUEUE_FLAG) == MGMT_USE_QUEUE_FLAG)
			free_num = GET_TXRING_FREENO(pAd, hwq_idx);
		else
			free_num = GET_MGMTRING_FREENO(pAd, hwq_idx);

#endif /* COMPOS_TESTMODE_WIN */

		if (multi_users > 0) {
			UCHAR *pate_pkt = TESTMODE_GET_PARAM(ate_ctrl, band_idx, pate_pkt);

			ate_ctrl->wcid_ref = multi_users;
			ret = MT_ATEGenPkt(pAd, pate_pkt, band_idx);
		}

		mode = TESTMODE_GET_PARAM(ate_ctrl, band_idx, Mode);

		if (mode & ATE_STOP)
			break;

		if (!(mode & ATE_TXFRAME))
			break;

		if (!free_num)
			break;

#if defined(MT7615) || defined(MT7622)
round_tx:

		if (((pkt_tx_time > 0) || (ipg > 0)) &&
			(pAd->mgmt_que.Number >= MGMT_QUE_MAX_NUMS))
			break;

#endif /* defined(MT7615) || defined(MT7622) */

		/* For ATE TX thread TX packet counter control */
		if (tx_cnt <= txed_cnt)
			break;

		ret = MT_ATETxPkt(pAd, band_idx);

		if (ret)
			break;

		txed_cnt++;
#if defined(MT7615) || defined(MT7622)
		if (((pkt_tx_time > 0) && need_ampdu) || (ipg > 0)) {
			PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *) pAd->PktTokenCb;
			UINT32 free_token_cnt =	pktTokenCb->tx_id_list.list->FreeTokenCnt;
			UINT32 pkt_tx_token_id_max = pktTokenCb->pkt_tx_tkid_max;

			free_num = GET_TXRING_FREENO(pAd, hwq_idx);

			if ((free_token_cnt > (pkt_tx_token_id_max - ATE_ENQUEUE_PACKET_NUM)) && (free_num > 0)) {
				if (pAd->mgmt_que.Number < MGMT_QUE_MAX_NUMS)
					goto round_tx;
			}
		}
#endif /* defined(MT7615) || defined(MT7622) */
		dequeue_size--;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s:: band_idx:%u, tx_cnt:%u, txed_cnt:%u, deque:%d, multi_user:%u, free:%lu\n",
			__func__, band_idx, tx_cnt, txed_cnt,
			dequeue_size, multi_users, free_num));

		if (!dequeue_size) {
			multi_users--;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s:: Dequeue %d finish, multi_user:%d\n",
				  __func__, dequeue_size, multi_users));
		} else
			goto ate_thread_dequeue;
	} while (multi_users > 0);

	TESTMODE_SET_PARAM(ate_ctrl, band_idx, TxedCount, txed_cnt);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, TxCount, tx_cnt);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:: band_idx:%u, tx_cnt:%u, txed_cnt:%u, deque:%d\n",
			  __func__, band_idx, tx_cnt, txed_cnt, dequeue_size));
	return ret;
}


static INT32 Mt_ATEThreadGetBandIdx(RTMP_ADAPTER *pAd, UINT8 *stat)
{
	UINT8 mask = 0;

	mask = 1 << TESTMODE_BAND0;

	if (*stat & mask) {
		*stat &= ~mask;
		return TESTMODE_BAND0;
	}

	mask = 1 << TESTMODE_BAND1;

	if (IS_ATE_DBDC(pAd) && (*stat & mask)) {
		*stat &= ~mask;
		return TESTMODE_BAND1;
	}

	return -1;
}

static INT TestMode_TxThread(ULONG Context)
{
	int status;
	INT32 ret = 0;
	RTMP_OS_TASK *pTask = (RTMP_OS_TASK *)Context;
	RTMP_ADAPTER *pAd = NULL;
	struct _ATE_CTRL *ate_ctrl = NULL;
	struct _ATE_TXTHREAD_CB *cb = NULL;
	INT32 band_idx = 0;
	UINT8 service_stat = 0;
	UINT32 mode = 0;

	if (!pTask)
		goto err1;

	pAd = (RTMP_ADAPTER *)RTMP_OS_TASK_DATA_GET(pTask);
	ate_ctrl = &pAd->ATECtrl;
	cb = &ate_ctrl->tx_thread[0];

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s: Init thread %u for band %u\n",
		__func__, ate_ctrl->current_init_thread, band_idx));
	RTMP_OS_COMPLETE(&ate_ctrl->cmd_done);

	while (!RTMP_OS_TASK_IS_KILLED(pTask)) {
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		service_stat = 0;
		RTMP_SEM_LOCK(&cb->lock);
		service_stat = cb->service_stat;

		/* AsicSetWPDMA(pAd, PDMA_TX_RX, 0); */
		do {
			if (!service_stat)
				break;

			band_idx = Mt_ATEThreadGetBandIdx(pAd, &service_stat);

			if (band_idx == -1)
				break;

			ret = MT_ATETxHandler(pAd, band_idx);
		} while (1);

		cb->service_stat = service_stat;
		/* AsicSetWPDMA(pAd, PDMA_TX_RX, 1); */
		RTMP_SEM_UNLOCK(&cb->lock);

		if (band_idx == -1)
			goto err1;

		mode = TESTMODE_GET_PARAM(ate_ctrl, band_idx, Mode);

		if (mode & fATE_MPS) {
			MT_ATEMPSRunStatCheck(pAd, band_idx);
			MT_ATEMPSLoadSetting(pAd, band_idx);
		}

		schedule();

		if (ret)
			break;
	}

err1:
	if (pTask)
		RtmpOSTaskNotifyToExit(pTask);
	if (pAd)
		MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, band_idx);
	cb->is_init = FALSE;

	if (ret)
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: abnormal leave err %d\n", __func__, ret));

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s: leave\n", __func__));
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
	return ret;
}


static INT32 TESTMODE_TXTHREAD_INIT(RTMP_ADAPTER *pAd, INT thread_idx)
{
	INT32 Ret = 0;
	CHAR thread_name[64] = "ATE_Thread";
	struct _ATE_CTRL *ate_ctrl = &pAd->ATECtrl;
	struct _ATE_TXTHREAD_CB *tx_cb = &pAd->ATECtrl.tx_thread[thread_idx];

	ate_ctrl->deq_cnt = 1;

	if (!ate_ctrl->tx_thread[thread_idx].is_init) {
		NdisZeroMemory(tx_cb, sizeof(*tx_cb));
		/* sprintf(thread_name, "ATE_Thread%d", thread_idx); */
		RTMP_OS_TASK_INIT(&tx_cb->task, thread_name, pAd);
		ate_ctrl->current_init_thread = thread_idx;
		NdisAllocateSpinLock(pAd, &tx_cb->lock);
		Ret =  RtmpOSTaskAttach(&tx_cb->task, TestMode_TxThread, (ULONG)&tx_cb->task);

		if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&ate_ctrl->cmd_done, ate_ctrl->cmd_expire))
			goto err0;

		if (Ret != STATUS_SUCCESS)
			goto err0;

		tx_cb->is_init = TRUE;
	} else {
		tx_cb->txed_cnt = 0;
		tx_cb->tx_cnt = 0;
	}

	tx_cb->service_stat = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: Initialize thread_idx=%d\n",
		__func__, thread_idx));
	return Ret;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: tx thread create fail\n", __func__));
	return Ret;
}


static INT32 TESTMODE_TXTHREAD_RELEASE(RTMP_ADAPTER *pAd, INT thread_idx)
{
	INT32 Ret = 0;
	struct _ATE_TXTHREAD_CB *tx_cb = &pAd->ATECtrl.tx_thread[thread_idx];

	if (&tx_cb->task)
		Ret = RtmpOSTaskKill(&tx_cb->task);

	if (Ret == NDIS_STATUS_FAILURE)
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("kill ATE Tx task failed!\n"));
	else
		tx_cb->is_init = FALSE;

	NdisFreeSpinLock(&tx_cb->lock);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: Release thread_idx=%d\n",
		__func__, thread_idx));
	return Ret;
}


static INT MT_ATEThreadSetService(RTMP_ADAPTER *pAd, UINT32 band_idx, UINT8 *stat)
{
	UINT8 mask = 0;

	if (IS_ATE_DBDC(pAd) && (band_idx == TESTMODE_BAND1))
		mask = 1 << TESTMODE_BAND1;
	else
		mask = 1 << TESTMODE_BAND0;

	*stat |= mask;
	return 0;
}


static VOID TESTMODEThreadProceedTx(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	struct _ATE_TXTHREAD_CB *tx_cb = NULL;
	tx_cb = &pAd->ATECtrl.tx_thread[0];
	RTMP_SEM_LOCK(&tx_cb->lock);
	MT_ATEThreadSetService(pAd, band_idx, &tx_cb->service_stat);
	RTMP_SEM_UNLOCK(&tx_cb->lock);
	RtmpOsTaskWakeUp(&tx_cb->task);
}


static VOID TESTMODEThreadStopTx(RTMP_ADAPTER *pAd, INT thread_idx)
{
	struct _ATE_TXTHREAD_CB *tx_cb = &pAd->ATECtrl.tx_thread[thread_idx];
	tx_cb->txed_cnt = 0;
	tx_cb->tx_cnt = 0;
}
#endif /* ATE_TXTHREAD */


VOID MT_ATEUpdateRxStatistic(RTMP_ADAPTER *pAd, enum _TESTMODE_STAT_TYPE type, VOID *data)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_RX_STATISTIC *rx_stat = &ATECtrl->rx_stat;
	UCHAR *uData = (UCHAR *)data;

	RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(uData + 8);
	RX_VECTOR1_3TH_CYCLE *RXV1_3TH_CYCLE = (RX_VECTOR1_3TH_CYCLE *)(uData + 16);
	RX_VECTOR1_5TH_CYCLE *RXV1_5TH_CYCLE = (RX_VECTOR1_5TH_CYCLE *)(uData + 24);
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
	RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(uData + 12);
	RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(uData + 20);

	/* RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(uData + 32); */
	RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(uData + 40);
#else
	RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(uData + 36);
	RX_VECTOR1_6TH_CYCLE *RXV1_6TH_CYCLE = (RX_VECTOR1_6TH_CYCLE *)(uData + 28);
#endif /* defined(MT7615) || defined(MT7622) */

	if (type == TESTMODE_RXV) {
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
		INT16 foe = 0;
		UINT32 i = 0;

		if (RXV1_1ST_CYCLE->TxMode == MODE_CCK) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\t%s: MODE_CCK RX\n", __func__));
			foe = (RXV1_5TH_CYCLE->MISC1 & 0x7ff);
			foe = (foe * 1000) >> 11;
		} else {
			UINT8 cbw = RXV1_1ST_CYCLE->FrMode;
			UINT32 foe_const = ((1 << (cbw + 1)) & 0xf) * 10000;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("\t%s: MODE_OFDM RX\n", __func__));
			foe = (RXV1_5TH_CYCLE->MISC1 & 0xfff);

			if (foe >= 2048)
				foe = foe - 4096;

			foe = (foe * foe_const) >> 15;
		}

		rx_stat->FreqOffsetFromRx = foe;
		rx_stat->RCPI[0] = RXV1_4TH_CYCLE->RCPI0;
		rx_stat->RCPI[1] = RXV1_4TH_CYCLE->RCPI1;
		rx_stat->RCPI[2] = RXV1_4TH_CYCLE->RCPI2;
		rx_stat->RCPI[3] = RXV1_4TH_CYCLE->RCPI3;
		rx_stat->RSSI[0] = (RXV1_4TH_CYCLE->RCPI0 >> 1) - 110;
		rx_stat->RSSI[1] = (RXV1_4TH_CYCLE->RCPI1 >> 1) - 110;
		rx_stat->RSSI[2] = (RXV1_4TH_CYCLE->RCPI2 >> 1) - 110;
		rx_stat->RSSI[3] = (RXV1_4TH_CYCLE->RCPI3 >> 1) - 110;
		rx_stat->FAGC_RSSI_IB[0] =
			RXV1_3TH_CYCLE->IBRssiRx;
		rx_stat->FAGC_RSSI_WB[0] =
			RXV1_3TH_CYCLE->WBRssiRx;
		rx_stat->FAGC_RSSI_IB[1] =
			RXV1_3TH_CYCLE->IBRssiRx;
		rx_stat->FAGC_RSSI_WB[1] =
			RXV1_3TH_CYCLE->WBRssiRx;
		rx_stat->FAGC_RSSI_IB[2] =
			RXV1_3TH_CYCLE->IBRssiRx;
		rx_stat->FAGC_RSSI_WB[2] =
			RXV1_3TH_CYCLE->WBRssiRx;
		rx_stat->FAGC_RSSI_IB[3] =
			RXV1_3TH_CYCLE->IBRssiRx;
		rx_stat->FAGC_RSSI_WB[3] =
			RXV1_3TH_CYCLE->WBRssiRx;
		rx_stat->SNR[0] = (RXV1_5TH_CYCLE->MISC1 >> 19) - 16;

		for (i = 0; i < 4; i++) {
			if (rx_stat->FAGC_RSSI_IB[i] >= 128)
				rx_stat->FAGC_RSSI_IB[i] -= 256;

			if (rx_stat->FAGC_RSSI_WB[i] >= 128)
				rx_stat->FAGC_RSSI_WB[i] -= 256;
		}

#ifdef CFG_SUPPORT_MU_MIMO

		if (RXV1_2ND_CYCLE->RxValidIndicator &&
			((RXV1_2ND_CYCLE->GroupId != 0) &&
			 (RXV1_2ND_CYCLE->GroupId != 63))) {
			rx_stat->RxMacMuPktCount++;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: get MU packet #%d\n",
				__func__, rx_stat->RxMacMuPktCount));
		}

#endif
		rx_stat->SIG_MCS = RXV1_1ST_CYCLE->TxRate;
		rx_stat->SINR = (*(UINT32 *)RXV2_3TH_CYCLE) & 0x00FFFFFF; /* bit[23:0] */
		rx_stat->RXVRSSI = RXV1_3TH_CYCLE->IBRssiRx;
#else
		rx_stat->FreqOffsetFromRx = RXV1_5TH_CYCLE->FoE;
		rx_stat->RCPI[0] = RXV1_3TH_CYCLE->Rcpi0;
		rx_stat->RCPI[1] = RXV1_3TH_CYCLE->Rcpi1;
		rx_stat->SNR[0] = RXV1_5TH_CYCLE->LTF_SNR0;
		rx_stat->SNR[1] = RXV2_2ND_CYCLE->OfdmLtfSNR1;
		rx_stat->RSSI[0] = RXV1_3TH_CYCLE->Rcpi0 / 2 - 110;
		rx_stat->RSSI[1] = RXV1_3TH_CYCLE->Rcpi1 / 2 - 110;
#endif /* defined(MT7615) || defined(MT7622) */

		if (pAd->ATECtrl.en_log & fATE_LOG_RXV)
			MT_ATEInsertLog(pAd, data, fATE_LOG_RXV, sizeof(struct _ATE_RXV_LOG));

		if (pAd->ATECtrl.en_log & fATE_LOG_RXINFO) {
			UINT32 nsts = 0;
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
			nsts = RXV1_2ND_CYCLE->NstsField;
#else
			nsts = RXV1_6TH_CYCLE->NsTsField;
#endif
#if !defined(COMPOS_TESTMODE_WIN)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tPhyMode=%d(%s)\n",
				RXV1_1ST_CYCLE->TxMode, get_phymode_str(RXV1_1ST_CYCLE->TxMode)));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tMCS=%d\n", RXV1_1ST_CYCLE->TxRate));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tBW=%d\n", RXV1_1ST_CYCLE->FrMode));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSGI=%d\n", RXV1_1ST_CYCLE->HtShortGi));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tSTBC=%d\n", RXV1_1ST_CYCLE->HtStbc));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tLDPC=%d\n", RXV1_1ST_CYCLE->HtAdCode));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNsts=%d\n", nsts));
#endif /* !defined(COMPOS_TESTMODE_WIN) */
		} else
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: EN_LOG:%x\n", __func__, pAd->ATECtrl.en_log));
	} else if (type == TESTMODE_RESET_CNT) {
		NdisZeroMemory(rx_stat, sizeof(*rx_stat));
		rx_stat->MaxRssi[0] = 0xff;
		rx_stat->MaxRssi[1] = 0xff;
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
		rx_stat->MaxRssi[2] = 0xff;
		rx_stat->MaxRssi[3] = 0xff;
#endif
	} else if (type == TESTMODE_COUNTER_802_11) {
		COUNTER_802_11 *wlanCounter = (COUNTER_802_11 *)data;
		rx_stat->RxMacFCSErrCount =
			wlanCounter->FCSErrorCount.u.LowPart;
	}
}


INT MtTestModeBkCr(PRTMP_ADAPTER pAd, ULONG offset, enum _TEST_BK_CR_TYPE type)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _TESTMODE_BK_CR *bks = ATECtrl->bk_cr;
	struct _TESTMODE_BK_CR *entry = NULL;
	INT32 i;

	if ((type >= TEST_BKCR_TYPE_NUM) || (type == TEST_EMPTY_BKCR))
		return NDIS_STATUS_INVALID_DATA;

	for (i = 0; i < MAX_TEST_BKCR_NUM; i++) {
		struct _TESTMODE_BK_CR *tmp = &bks[i];

		if ((tmp->type == TEST_EMPTY_BKCR) && (entry == NULL)) {
			entry = tmp;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s: find emptyp bk entry %d\n", __func__, i));
		} else if ((tmp->type == type) && (tmp->offset == offset)) {
			entry = tmp;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s: update bk entry %d\n", __func__, i));
			break;
		}
	}

	if (!entry)
		return NDIS_STATUS_RESOURCES;

	entry->type = type;
	entry->offset = offset;

	switch (type) {
	case TEST_MAC_BKCR:
		MAC_IO_READ32(pAd, offset, &entry->val);
		break;

	case TEST_HIF_BKCR:
		HIF_IO_READ32(pAd, offset, &entry->val);
		break;

	case TEST_PHY_BKCR:
		PHY_IO_READ32(pAd, offset, &entry->val);
		break;

	case TEST_HW_BKCR:
		HW_IO_READ32(pAd, offset, &entry->val);
		break;

	case TEST_MCU_BKCR:
		MCU_IO_READ32(pAd, offset, &entry->val);
		break;

	default:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("%s: bk-type not supported\n", __func__));
		entry->type = TEST_EMPTY_BKCR;
		entry->offset = 0;
		break;
	}

	return NDIS_STATUS_SUCCESS;
}


INT MtTestModeRestoreCr(PRTMP_ADAPTER pAd, ULONG offset)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _TESTMODE_BK_CR *bks = ATECtrl->bk_cr;
	struct _TESTMODE_BK_CR *entry = NULL;
	INT32 i;

	for (i = 0; i < MAX_TEST_BKCR_NUM; i++) {
		struct _TESTMODE_BK_CR *tmp = &bks[i];

		if (tmp->offset == offset) {
			entry = tmp;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s: find entry %d\n", __func__, i));
			break;
		}
	}

	if (!entry)
		return NDIS_STATUS_INVALID_DATA;

	switch (entry->type) {
	case TEST_MAC_BKCR:
		MAC_IO_WRITE32(pAd, offset, entry->val);
		break;

	case TEST_HIF_BKCR:
		HIF_IO_WRITE32(pAd, offset, entry->val);
		break;

	case TEST_PHY_BKCR:
		PHY_IO_WRITE32(pAd, offset, entry->val);
		break;

	case TEST_HW_BKCR:
		HW_IO_WRITE32(pAd, offset, entry->val);
		break;

	case TEST_MCU_BKCR:
		MCU_IO_WRITE32(pAd, offset, entry->val);
		break;

	default:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("%s: bk-type not supported\n", __func__));
		entry->type = TEST_EMPTY_BKCR;
		entry->offset = 0;
		break;
	}

	entry->type = TEST_EMPTY_BKCR;
	entry->offset = 0;
	entry->val = 0;
	return NDIS_STATUS_SUCCESS;
}


static INT32 MtATEPayloadInit(RTMP_ADAPTER *pAd, UCHAR *pPacket, UINT32 len, UINT32 band_idx)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UINT32 policy = TESTMODE_GET_PARAM(ATECtrl, band_idx, FixedPayload);
	UCHAR *payload = TESTMODE_GET_PARAM(ATECtrl, band_idx, payload);
	UINT32 pl_len = TESTMODE_GET_PARAM(ATECtrl, band_idx, pl_len);
	UINT32 pos = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: len:%d, band_idx:%u, len:%u, pl_len:%u, policy:%x\n",
		__func__, len, band_idx, len, pl_len, policy));

	if (policy == ATE_RANDOM_PAYLOAD) {
		for (pos = 0; pos < len; pos++)
			pPacket[pos] = RandomByte(pAd);

		return 0;
	}

	if (!payload)
		return NDIS_STATUS_FAILURE;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: payload:%x\n", __func__, payload[0]));

	if (pl_len == 0) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Payload length can't be 0!!\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	if (policy == ATE_USER_PAYLOAD) {
		NdisZeroMemory(pPacket, len);
		NdisMoveMemory(pPacket, payload, pl_len);
	} else if (policy == ATE_FIXED_PAYLOAD) {
		for (pos = 0; pos < len; pos += pl_len)
			NdisMoveMemory(&pPacket[pos], payload, pl_len);
	}

	return 0;
}






static INT32 MT_ATESetTxPowerX(RTMP_ADAPTER *pAd, ATE_TXPOWER TxPower)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UINT32 Channel = TESTMODE_GET_PARAM(ATECtrl, TxPower.Dbdc_idx, Channel);
	UINT32 Ch_Band = TESTMODE_GET_PARAM(ATECtrl, TxPower.Dbdc_idx, Ch_Band);

	if (TxPower.Channel == 0)
		TxPower.Channel = Channel;

	TxPower.Band_idx = Ch_Band ? Ch_Band : TxPower.Band_idx;

	if (TxPower.Channel > 14)
		TxPower.Band_idx = 1; /* 5G band */

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Channel:%d Power:%x Ch_Band:%d Ant:%d\n",
		__func__, TxPower.Channel, TxPower.Power, TxPower.Band_idx, TxPower.Ant_idx));

	if (TxPower.Power <= 0)
		return Ret;

	switch (TxPower.Ant_idx) {
	case 0:
		ATECtrl->TxPower0 = TxPower.Power;
		break;

	case 1:
		ATECtrl->TxPower1 = TxPower.Power;
		break;

	case 2:
		ATECtrl->TxPower2 = TxPower.Power;
		break;

	case 3:
		ATECtrl->TxPower3 = TxPower.Power;
		break;

	default:
		break;
	}

	Ret = MtCmdSetTxPowerCtrl(pAd, TxPower);
	return Ret;
}


static INT32 MT_ATESetTxPower0(RTMP_ADAPTER *pAd, ATE_TXPOWER TxPower)
{
	INT32 Ret = 0;
	TxPower.Ant_idx = 0;
	Ret = MT_ATESetTxPowerX(pAd, TxPower);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	return Ret;
}


static INT32 MT_ATESetTxPower1(RTMP_ADAPTER *pAd, ATE_TXPOWER TxPower)
{
	INT32 Ret = 0;
	TxPower.Ant_idx = 1;
	Ret = MT_ATESetTxPowerX(pAd, TxPower);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	return Ret;
}


static INT32 MT_ATESetTxPower2(RTMP_ADAPTER *pAd, ATE_TXPOWER TxPower)
{
	INT32 Ret = 0;
	TxPower.Ant_idx = 2;
	Ret = MT_ATESetTxPowerX(pAd, TxPower);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	return Ret;
}


static INT32 MT_ATESetTxPower3(RTMP_ADAPTER *pAd, ATE_TXPOWER TxPower)
{
	INT32 Ret = 0;
	TxPower.Ant_idx = 3;
	Ret = MT_ATESetTxPowerX(pAd, TxPower);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	return Ret;
}

static INT32 MT_ATESetForceTxPower(RTMP_ADAPTER *pAd, INT8 cTxPower, UINT8 ucPhyMode, UINT8 ucTxRate, UINT8 ucBW)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	/* update related Tx parameters */
	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, PhyMode, ucPhyMode);
	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, Mcs, ucTxRate);
	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, BW, ucBW);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Band(%d), TxMode(%d), MCS(%d), BW(%d), TxPower(%d)\n",
		__FUNCTION__, control_band_idx, ucPhyMode, ucTxRate, ucBW, cTxPower));

	/* firmware command for Force Tx Power Conrtrol */
	MtCmdSetForceTxPowerCtrl(pAd, control_band_idx, cTxPower, ucPhyMode, ucTxRate, ucBW);

	return Ret;
}

#ifdef LOGDUMP_TO_FILE
static INT32 MT_ATEDumpReCal(struct _ATE_LOG_DUMP_ENTRY entry, INT idx, RTMP_OS_FD_EXT srcf)
#else
static INT32 MT_ATEDumpReCal(struct _ATE_LOG_DUMP_ENTRY entry, INT idx)
#endif
{
	struct _ATE_LOG_RECAL re_cal = entry.log.re_cal;
	INT32 ret = 0;
#ifdef LOGDUMP_TO_FILE
	INT len = 7 + 2 * 3 + 8 * 3 + 1;
	CHAR msg[len];
	os_zero_mem(msg, len);
#endif
	/* MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, */
	/* ("CalType:%x ", re_cal.cal_type)); */
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[Recal][%08x][%08x]%08x\n", re_cal.cal_type, re_cal.cr_addr, re_cal.cr_val));
	/* MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, */
	/* ("%08x\n", re_cal.cr_val)); */
#ifdef LOGDUMP_TO_FILE
	sprintf(msg, "[Recal][%08x][%08x]%08x\n", re_cal.cal_type, re_cal.cr_addr, re_cal.cr_val);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Length:%d %s\n", strlen(msg), msg));
	MT_ATEWriteFd(msg, srcf);
#endif
	return ret;
}

static INT32 MT_ATEInsertReCal(struct _ATE_LOG_DUMP_ENTRY *entry, UCHAR *data, UINT32 len)
{
	struct _ATE_LOG_RECAL *re_cal = NULL;
	INT32 ret = 0;

	if (!entry)
		goto err0;

	if (!data)
		goto err0;

	os_zero_mem(entry, sizeof(*entry));
	entry->log_type = fATE_LOG_RE_CAL;
	entry->un_dumped = TRUE;
	re_cal = &entry->log.re_cal;
	NdisMoveMemory((UINT8 *)re_cal, data,
				   sizeof(*re_cal));
	return ret;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: NULL entry %p, data %p\n",
		__func__, entry, data));
	return NDIS_STATUS_FAILURE;
}


static INT32 MT_ATEInsertRDD(struct _ATE_LOG_DUMP_ENTRY *entry, UCHAR *data, UINT32 len)
{
	INT ret = 0;

	if (!entry)
		goto err0;

	if (!data)
		goto err0;

	os_zero_mem(entry, sizeof(*entry));
	entry->log_type = fATE_LOG_RDD;
	entry->un_dumped = TRUE;

	if (len > sizeof(entry->log.rdd))
		len = sizeof(entry->log.rdd);

	NdisMoveMemory((UCHAR *)&entry->log.rdd, data, len);
	return ret;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: NULL entry %p, data %p\n",
		__func__, entry, data));
	return -1;
}


static INT32 MT_ATEInsertRXV(struct _ATE_LOG_DUMP_ENTRY *entry, UCHAR *data, UINT32 len)
{
	RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(data + 8);
	RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(data + 12);
	RX_VECTOR1_3TH_CYCLE *RXV1_3TH_CYCLE = (RX_VECTOR1_3TH_CYCLE *)(data + 16);
	RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(data + 20);
	RX_VECTOR1_5TH_CYCLE *RXV1_5TH_CYCLE = (RX_VECTOR1_5TH_CYCLE *)(data + 24);
	RX_VECTOR1_6TH_CYCLE *RXV1_6TH_CYCLE = (RX_VECTOR1_6TH_CYCLE *)(data + 28);
	RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(data + 32);
	RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(data + 36);
	RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(data + 40);

	if (!entry)
		goto err0;

	if (!data)
		goto err0;

	os_zero_mem(entry, sizeof(*entry));
	entry->log_type = fATE_LOG_RXV;
	entry->un_dumped = TRUE;

	if (RXV1_1ST_CYCLE)
		entry->log.rxv.rxv1_1st = *RXV1_1ST_CYCLE;

	if (RXV1_2ND_CYCLE)
		entry->log.rxv.rxv1_2nd = *RXV1_2ND_CYCLE;

	if (RXV1_3TH_CYCLE)
		entry->log.rxv.rxv1_3rd = *RXV1_3TH_CYCLE;

	if (RXV1_4TH_CYCLE)
		entry->log.rxv.rxv1_4th = *RXV1_4TH_CYCLE;

	if (RXV1_5TH_CYCLE)
		entry->log.rxv.rxv1_5th = *RXV1_5TH_CYCLE;

	if (RXV1_6TH_CYCLE)
		entry->log.rxv.rxv1_6th = *RXV1_6TH_CYCLE;

	if (RXV2_1ST_CYCLE)
		entry->log.rxv.rxv2_1st = *RXV2_1ST_CYCLE;

	if (RXV2_2ND_CYCLE)
		entry->log.rxv.rxv2_2nd = *RXV2_2ND_CYCLE;

	if (RXV2_3TH_CYCLE)
		entry->log.rxv.rxv2_3rd = *RXV2_3TH_CYCLE;

	return 0;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: NULL entry %p, data %p\n",
		__func__, entry, data));
	return -1;
}


#ifdef LOGDUMP_TO_FILE
static INT32 MT_ATEDumpRXV(struct _ATE_LOG_DUMP_ENTRY entry, INT idx, RTMP_OS_FD_EXT srcf)
#else
static INT32 MT_ATEDumpRXV(struct _ATE_LOG_DUMP_ENTRY entry, INT idx)
#endif
{
	INT32 ret = 0;
	struct _ATE_RXV_LOG log = entry.log.rxv;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%%[RXV DUMP START][%d]\n", idx));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[RXVD1]%08x\n", *((UINT32 *)&log.rxv1_1st)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[RXVD2]%08x\n", *((UINT32 *)&log.rxv1_2nd)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[RXVD3]%08x\n", *((UINT32 *)&log.rxv1_3rd)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[RXVD4]%08x\n", *((UINT32 *)&log.rxv1_4th)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[RXVD5]%08x\n", *((UINT32 *)&log.rxv1_5th)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[RXVD6]%08x\n", *((UINT32 *)&log.rxv1_6th)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[RXVD7]%08x\n", *((UINT32 *)&log.rxv2_1st)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[RXVD8]%08x\n", *((UINT32 *)&log.rxv2_2nd)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[RXVD9]%08x\n", *((UINT32 *)&log.rxv2_3rd)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[RXV DUMP END]\n"));
	return ret;
}

#ifdef LOGDUMP_TO_FILE
static INT MT_ATEWriteFd(RTMP_STRING *log, RTMP_OS_FD_EXT srcf)
{
	INT ret = 0;
	INT len = strlen(log);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: Write len %d\n", __func__, len));
	ret = os_file_write(srcf, log, len);
	return ret;
}


static RTMP_OS_FD_EXT MT_ATEGetFileFd(UINT32 log_type, INT idx)
{
	RTMP_STRING src[64];
	RTMP_OS_FD_EXT srcf;

	switch (log_type) {
	case ATE_LOG_RXV:
		sprintf(src, "RXVDump_v%d.txt", idx);
		break;

	case ATE_LOG_RDD:
		sprintf(src, "RDDDump_v%d.txt", idx);
		break;

	case ATE_LOG_RE_CAL:
		sprintf(src, "RECALDump_v%08x.txt", (UINT32)idx);
		break;

	default:
		srcf.Status = NDIS_STATUS_FAILURE;
		goto err0;
	}

	/* srcf = os_file_open(src, O_WRONLY|O_CREAT|O_TRUNC, 0); */
	srcf = os_file_open(src, O_WRONLY | O_CREAT | O_APPEND, 0);
	return srcf;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown log type %08x\n",
		__func__, log_type));
	return srcf;
}


static INT32 MT_ATEReleaseLogFd(RTMP_OS_FD_EXT *srcf)
{
	UCHAR ret = 0;

	if (os_file_close(*srcf) != 0)
		goto err0;

	return ret;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Error closing file\n", __func__));
	return NDIS_STATUS_FAILURE;
}
#endif


#if defined(COMPOS_TESTMODE_WIN)
static INT32 MT_ATEDumpRXVToFile(RTMP_ADAPTER *pAd, struct _ATE_LOG_DUMP_CB *log_cb, UINT32 idx)
{
	/* todo: check this function work properly under all the case. */
	UINT32 copyIndex = 0;
	KIRQL oldIrql;
	UCHAR tempBuffer[512];
	struct _ATE_RXV_LOG log;
	struct _ATE_LOG_DUMP_ENTRY *entry;
	UCHAR *writeBuffer = (UCHAR *)WINAllocateMemory(CALIBRATION_BUFFER_SIZE);
	RTMPMoveMemory(writeBuffer, "[LOG DUMP START]\n", strlen("[LOG DUMP START]\n"));
	copyIndex += strlen("[LOG DUMP START]\n");

	do {
		if (log_cb->entry[idx].un_dumped) {
			entry = &log_cb->entry[idx];
			log = entry->log.rxv;
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer),
							   "[%d]", idx);
			RTMPMoveMemory(&writeBuffer[copyIndex],
						   tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RTMPMoveMemory(&writeBuffer[copyIndex], "[RXV DUMP START]\r\n", strlen("[RXV DUMP START]\r\n"));
			copyIndex += strlen("[RXV DUMP START]\r\n");
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD1]%08x\r\n", *((UINT32 *)&log.rxv1_1st));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD2]%08x\r\n", *((UINT32 *)&log.rxv1_2nd));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD3]%08x\r\n", *((UINT32 *)&log.rxv1_3rd));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD4]%08x\r\n", *((UINT32 *)&log.rxv1_4th));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD5]%08x\r\n", *((UINT32 *)&log.rxv1_5th));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD6]%08x\r\n", *((UINT32 *)&log.rxv1_6th));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD7]%08x\r\n", *((UINT32 *)&log.rxv2_1st));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD8]%08x\r\n", *((UINT32 *)&log.rxv2_2nd));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RtlStringCbPrintfA(tempBuffer, sizeof(tempBuffer), "[RXVD9]%08x\r\n", *((UINT32 *)&log.rxv2_3rd));
			RTMPMoveMemory(&writeBuffer[copyIndex], tempBuffer, strlen(tempBuffer));
			copyIndex += strlen(tempBuffer);
			RTMPMoveMemory(&writeBuffer[copyIndex], "[RXV DUMP END]\r\n", strlen("[RXV DUMP END]\r\n"));
			copyIndex += strlen("[RXV DUMP END]\r\n");
			log_cb->entry[idx].un_dumped = FALSE;
		}

		INC_RING_INDEX(idx, log_cb->len);
	} while (idx != log_cb->idx);

	RTMPMoveMemory(&writeBuffer[copyIndex], "[LOG DUMP END]\r\n", strlen("[LOG DUMP END]\r\n"));
	copyIndex += strlen("[LOG DUMP END]\r\n");

	if (pAd->ReCalibrationSize + copyIndex <
		CALIBRATION_BUFFER_SIZE) {
		KeAcquireSpinLock(&pAd->RxVectorDumpLock,
						  &oldIrql);
		RTMPMoveMemory(
			&pAd->RxVectorDumpBuffer[pAd->RxVectorDumpSize],
			writeBuffer, copyIndex);
		pAd->RxVectorDumpSize += copyIndex;
		KeReleaseSpinLock(&pAd->RxVectorDumpLock,
						  oldIrql);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RxVectorDumpSize = 0x%x, Dump size = 0x%x\n",
				  pAd->RxVectorDumpSize, copyIndex));

		if (copyIndex > 0 && pAd->bIsCalDumpThreadRunning) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", pAd->ReCalibrationBuffer));
			KeSetEvent(&pAd->WriteEvent, 0, FALSE);
		} else
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Calibration buffer 0x%x + dump size 0x%x over Limit 0x%x\n",
					  pAd->RxVectorDumpSize, copyIndex, CALIBRATION_BUFFER_SIZE));
	}

	WINFreeMemory(writeBuffer);
	writeBuffer = NULL;
}


#endif /*#if defined(COMPOS_TESTMODE_WIN)*/


INT32 MT_ATEInsertLog(RTMP_ADAPTER *pAd, UCHAR *log, UINT32 log_type, UINT32 len)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_LOG_DUMP_CB *log_cb = NULL;
	INT idx = 0;
	INT logcb_idx = 0;
	UINT32 is_dumping = 0;
	INT32 (*insert_func)(struct _ATE_LOG_DUMP_ENTRY *entry, UCHAR *data, UINT32 len) = NULL;

	switch (log_type) {
	case fATE_LOG_RXV:
		insert_func = MT_ATEInsertRXV;
		logcb_idx = ATE_LOG_RXV - 1;
		break;

	case fATE_LOG_RDD:
		insert_func = MT_ATEInsertRDD;
		logcb_idx = ATE_LOG_RDD - 1;
		break;

	case fATE_LOG_RE_CAL:
		insert_func = MT_ATEInsertReCal;
		logcb_idx = ATE_LOG_RE_CAL - 1;
		break;

	default:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown log type %08x\n", __func__, log_type));
		break;
	}

	if (!insert_func)
		goto err0;

	log_cb = &ATECtrl->log_dump[logcb_idx];
	idx = log_cb->idx;
	OS_SPIN_LOCK(&log_cb->lock);
	is_dumping = log_cb->is_dumping;
	OS_SPIN_UNLOCK(&log_cb->lock);

	if (is_dumping)
		goto err1;

	if ((log_cb->idx + 1) == log_cb->len) {
		if (!log_cb->overwritable)
			goto err0;
		else
			log_cb->is_overwritten = TRUE;
	}

	OS_SPIN_LOCK(&log_cb->lock);

	if (log_cb->entry == NULL) {
		OS_SPIN_UNLOCK(&log_cb->lock);
		goto err0;
	}

	ret = insert_func(&log_cb->entry[idx], log, len);
	OS_SPIN_UNLOCK(&log_cb->lock);

	if (ret)
		goto err0;

	INC_RING_INDEX(log_cb->idx, log_cb->len);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_LOUD, ("%s: idx:%d, log_cb->idx:%d, log_type:%08x\n",
			  __func__, idx, log_cb->idx, log_type));
	return ret;
err0:
	if (log_cb)
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("[WARN]%s: idx:%x, overwritable:%x, log_type:%08x\n",
			__func__, idx, log_cb->overwritable, log_type));
err1:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: Log dumping\n", __func__));
	return -NDIS_STATUS_RESOURCES;
}


INT32 MT_ATEDumpLog(RTMP_ADAPTER *pAd, struct _ATE_LOG_DUMP_CB *log_cb, UINT32 log_type)
{
	INT32 ret = 0;
	INT idx = 0;
	UINT32 u4BufferCounter = 0;
#ifdef LOGDUMP_TO_FILE
	INT32 (*dump_func)(struct _ATE_LOG_DUMP_ENTRY, INT idx, RTMP_OS_FD_EXT fd) = NULL;
#else
	INT32 (*dump_func)(struct _ATE_LOG_DUMP_ENTRY, INT idx) = NULL;
#endif
	INT debug_lvl = DebugLevel;
	CHAR Log_type[64];
#ifdef LOGDUMP_TO_FILE
	INT len = 5 + 2 * 3 + 5 + 1;
	CHAR msg[len];
	os_zero_mem(msg, len);
#endif

	if (!log_cb->entry)
		goto err0;

	/* For QAtool log buffer limitation. We should record the current index for next function called.  */
	if (pAd->fgQAtoolBatchDumpSupport)
		idx = pAd->u2LogEntryIdx;

	if (log_cb->is_overwritten)
		idx = log_cb->idx;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: idx:%d, log_type:%08x, log_cb->idx:%d\n", __func__, idx, log_type, log_cb->idx));

	switch (log_type) {
	case ATE_LOG_RXV:
		dump_func = MT_ATEDumpRXV;
		strcpy(Log_type, "LOG");
		break;

	case ATE_LOG_RDD:
		dump_func = MT_ATERDDParseResult;
		strcpy(Log_type, "RDD");
		break;

	case ATE_LOG_RE_CAL:
		dump_func = MT_ATEDumpReCal;
		strcpy(Log_type, "RECAL");
		break;

	default:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown log type %08x\n", __func__, log_type));
		break;
	}

	if (!dump_func)
		goto err0;

#ifdef LOGDUMP_TO_FILE

	if (log_type == ATE_LOG_RE_CAL)
		log_cb->fd = MT_ATEGetFileFd(log_type, log_cb->recal_curr_type);
	else
		log_cb->fd = MT_ATEGetFileFd(log_type, log_cb->file_idx);

	if (log_cb->fd.Status)
		goto err1;

	if (log_type == ATE_LOG_RE_CAL) {
		sprintf(msg, "[Recal][%08x][START]\n", log_cb->recal_curr_type);
		MT_ATEWriteFd(msg, log_cb->fd);
	}

#endif
	DebugLevel = DBG_LVL_OFF;
	OS_SPIN_LOCK(&log_cb->lock);
	log_cb->is_dumping = TRUE;
	OS_SPIN_UNLOCK(&log_cb->lock);
#if defined(COMPOS_TESTMODE_WIN)
	/* dump RX vector to file */
	CreateThread(pAd);
	MT_ATEDumpRXVToFile(pAd, log_cb, idx);
#else
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s DUMP START]\n", Log_type));
	pAd->fgDumpStart = 1;

	do {
		if (log_cb->entry[idx].un_dumped) {
			/* MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%d]", idx)); */
#ifdef LOGDUMP_TO_FILE
			dump_func(log_cb->entry[idx], idx, log_cb->fd);
#else
			dump_func(log_cb->entry[idx], idx);
#endif
			log_cb->entry[idx].un_dumped = FALSE;
			u4BufferCounter++;
		}

		/* The size of per entry is 38 bytes and for QAtool log buffer limitation. */
		if ((pAd->fgQAtoolBatchDumpSupport) &&
			(u4BufferCounter >= (1 << (CONFIG_LOG_BUF_SHIFT - 1)) / 38)) {
			pAd->u2LogEntryIdx = idx;
			break;
		}

		INC_RING_INDEX(idx, log_cb->len);
	} while (idx != log_cb->idx);

#ifdef LOGDUMP_TO_FILE

	if (log_type == ATE_LOG_RE_CAL) {
		sprintf(msg, "[Recal][%08x][END]\n", log_cb->recal_curr_type);
		MT_ATEWriteFd(msg, log_cb->fd);
	}

#endif

	if ((idx == log_cb->idx) && (pAd->fgDumpStart)) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s DUMP END]\n", Log_type));
		pAd->fgDumpStart = 0;
	}

#endif
	OS_SPIN_LOCK(&log_cb->lock);
	log_cb->is_dumping = FALSE;
	OS_SPIN_UNLOCK(&log_cb->lock);
	DebugLevel = debug_lvl;
#ifdef LOGDUMP_TO_FILE
	MT_ATEReleaseLogFd(&log_cb->fd);
	log_cb->file_idx++;
#endif
	return ret;
#ifdef LOGDUMP_TO_FILE
err1:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Error opening log file\n", __func__));
#endif
err0:
	return -1;
}


static INT32 MT_ATEInitLogCB(RTMP_ADAPTER *pAd, struct _ATE_LOG_DUMP_CB *log_cb, UINT32 size, UCHAR overwrite)
{
	INT32 ret = 0;

	if (!log_cb->entry) {
		NdisZeroMemory(log_cb, sizeof(*log_cb));
		ret = os_alloc_mem(pAd, (PUCHAR *)&log_cb->entry, size * sizeof(struct _ATE_LOG_DUMP_ENTRY));

		if (ret)
			goto err0;

		os_zero_mem(log_cb->entry, size * sizeof(struct _ATE_LOG_DUMP_ENTRY));
		log_cb->len = size;
		NdisAllocateSpinLock(pAd, &log_cb->lock);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s-v4, init log cb size %u, log_cb->len:%u\n", __func__, size, log_cb->len));
	}

	log_cb->overwritable = overwrite;
	log_cb->is_overwritten = FALSE;
	log_cb->idx = 0;
#ifdef LOGDUMP_TO_FILE
	log_cb->file_idx = 0;
#endif
	return ret;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: Alcated memory fail! size %u\n", __func__, size));
	return ret;
}

static INT32 MT_ATEReleaseLogDump(RTMP_ADAPTER *pAd)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ate_ctrl = &pAd->ATECtrl;
	struct _ATE_LOG_DUMP_CB *log_cb = NULL;
	INT i = 0;
	ate_ctrl->en_log = 0;

	for (i = 0; i < (ATE_LOG_TYPE_NUM - 1); i++) {
		log_cb = &ate_ctrl->log_dump[i];

		if (log_cb->entry) {
			os_free_mem(log_cb->entry);
			NdisFreeSpinLock(&log_cb->lock);
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: release log cb type %d\n", __func__, i + 1));
		}
	}

	return ret;
}


static VOID MT_ATEAirTimeOnOff(struct _RTMP_ADAPTER *pAd, BOOLEAN Enable)
{
	UINT32 Value;

	if (!Enable) {
		MAC_IO_READ32(pAd, RMAC_AIRTIME0, &Value);
		Value = Value & (~RX_AIRTIME_EN);
		MAC_IO_WRITE32(pAd, RMAC_AIRTIME0, Value);
	}
}


static INT32 MT_ATELogOnOff(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 on_off, UINT32 size)
{
	INT ret = 0;
	UINT32 mask = 0;
	UCHAR overwrite = TRUE;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_LOG_DUMP_CB *log_cb = NULL;

	switch (type) {
	case ATE_LOG_RXV:
		mask = fATE_LOG_RXV;
		/* Disable RX airtime function to avoid affecting RXV2 Cycle3 */
		/* HW design of RXV2 Cycle3 will be shared for RXV debug function and RX airtime funciotn. */
		MT_ATEAirTimeOnOff(pAd, 0);
		break;

	case ATE_LOG_RDD:
		overwrite = FALSE;
		mask = fATE_LOG_RDD;
		break;

	case ATE_LOG_RE_CAL:
		/* size = ATE_RECAL_LOG_SIZE; */
		mask = fATE_LOG_RE_CAL;
		break;

	case ATE_LOG_RXINFO:
		mask = fATE_LOG_RXINFO;
		break;

	case ATE_LOG_TXDUMP:
		mask = fATE_LOG_TXDUMP;
		break;

	case ATE_LOG_TEST:
		mask = fATE_LOG_TEST;
		break;

	default:
		goto err0;
	}

	if (type < ATE_LOG_TYPE_NUM)
		log_cb = &ATECtrl->log_dump[type - 1];

	if (on_off == ATE_LOG_ON) {
		if (log_cb)
			ret = MT_ATEInitLogCB(pAd, log_cb, size, overwrite);

		if (ret)
			goto err1;

		ATECtrl->en_log |= mask;

		if (pAd->fgQAtoolBatchDumpSupport)
			pAd->u2LogEntryIdx = 0;
	} else if (on_off == ATE_LOG_OFF) {
		ATECtrl->en_log &= ~mask;

		if (pAd->fgQAtoolBatchDumpSupport)
			pAd->u2LogEntryIdx = 0;
	} else if (on_off == ATE_LOG_DUMP) {
		if (log_cb)
			ret = MT_ATEDumpLog(pAd, log_cb, type);

		if (ret)
			goto err1;
	} else
		goto err2;

	return ret;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s log type %d not supported\n", __func__, type));
err1:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s log type %d init logCB fail\n", __func__, type));
err2:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s log ctrl %d not supported\n", __func__, on_off));
	return -1;
}


static INT32 MT_ATESetICapStart(
				RTMP_ADAPTER *pAd,
				BOOLEAN Trigger,
				BOOLEAN RingCapEn,
				UINT32 Event,
				UINT32 Node,
				UINT32 Len,
				UINT32 StopCycle,
				UINT32 BW,
				UINT32 MACTriggerEvent,
				UINT32 SourceAddrLSB,
				UINT32 SourceAddrMSB,
				UINT32 Band)
{
	INT32 Ret = 0;
#ifdef INTERNAL_CAPTURE_SUPPORT
	INT32 retval = 0;
	RBIST_CAP_START_T *prICapInfo = NULL;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);
#endif/* INTERNAL_CAPTURE_SUPPORT */

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

#ifdef INTERNAL_CAPTURE_SUPPORT
	retval = os_alloc_mem(pAd, (UCHAR **)&prICapInfo, sizeof(RBIST_CAP_START_T));
	if (retval != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : Not enough memory for dynamic allocating !!\n", __func__));
		goto error;
	}
	os_zero_mem(prICapInfo, sizeof(RBIST_CAP_START_T));

	prICapInfo->fgTrigger = Trigger;
	prICapInfo->fgRingCapEn = RingCapEn;
	prICapInfo->u4TriggerEvent = Event;
	prICapInfo->u4CaptureNode = Node;
	prICapInfo->u4CaptureLen = Len;
	prICapInfo->u4CapStopCycle = StopCycle;
	prICapInfo->u4BW = BW;
	prICapInfo->u4MACTriggerEvent = MACTriggerEvent;
	prICapInfo->u4SourceAddressLSB = SourceAddrLSB;
	prICapInfo->u4SourceAddressMSB = SourceAddrMSB;
	prICapInfo->u4BandIdx = Band;

	if (ops->ICapStart != NULL)
		Ret = ops->ICapStart(pAd, (UINT8 *)prICapInfo);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : The function is not hooked !!\n", __func__));
	}

error:
	if (prICapInfo != NULL)
		os_free_mem(prICapInfo);
#endif/* INTERNAL_CAPTURE_SUPPORT */

	return Ret;
}


static INT32 MT_ATEGetICapStatus(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
#ifdef INTERNAL_CAPTURE_SUPPORT
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	if (ops->ICapStatus != NULL) {
		Ret = ops->ICapStatus(pAd);

		if (IS_MT7615(pAd)) {
			UINT32 StartAddr1, StartAddr2, StartAddr3, EndAddr;
			UINT32 StopAddr, Wrap;

			RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&ate_ctrl->cmd_done, ate_ctrl->cmd_expire);
			StartAddr1 = ate_ctrl->icap_info.u4StartAddr1;
			StartAddr2 = ate_ctrl->icap_info.u4StartAddr2;
			StartAddr3 = ate_ctrl->icap_info.u4StartAddr3;
			EndAddr = ate_ctrl->icap_info.u4EndAddr;
			StopAddr = ate_ctrl->icap_info.u4StopAddr;
			Wrap = ate_ctrl->icap_info.u4Wrap;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, StartAddr1:%02x StartAddr2:%02x StartAddr3:%02x EndAddr:%02x StopAddr:%02x Wrap:%02x\n",
					__func__, StartAddr1, StartAddr2, StartAddr3, EndAddr, StopAddr, Wrap));
		}
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : The function is not hooked !!\n", __func__));
	}
#endif/* INTERNAL_CAPTURE_SUPPORT */

	return Ret;
}


static INT32 MT_ATEGetICapIQData(RTMP_ADAPTER
								 *pAd,
								 PINT32 pData,
								 PINT32 pDataLen,
								 UINT32 IQ_Type,
								 UINT32 WF_Num)
{
	INT32 Ret = 0;
#ifdef INTERNAL_CAPTURE_SUPPORT
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	if (ops->ICapGetIQData != NULL)
		Ret = ops->ICapGetIQData(pAd, pData, pDataLen, IQ_Type, WF_Num);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : The function is not hooked !!\n", __func__));
	}
#endif/* INTERNAL_CAPTURE_SUPPORT */

	return Ret;
}


#if !defined(COMPOS_TESTMODE_WIN)	/* 1todo too many OS private function */
INT MT_ATERxDoneHandle(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

		INC_COUNTER64(pAd->WlanCounters[0].ReceivedFragmentCount);

	{
		UINT32 chfreq0 = 0, chfreq1 = 0;
		union _RMAC_RXD_0_UNION *rxd_0;
		UINT32 pkt_type;
		RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)pRxBlk->rmac_info;

		rxd_0 = (union _RMAC_RXD_0_UNION *)pRxBlk->rmac_info;
		pkt_type = RMAC_RX_PKT_TYPE(rxd_0->word);

		switch (pkt_type) {
		case RMAC_RX_PKT_TYPE_RX_NORMAL:
			MAC_IO_READ32(pAd, RMAC_CHFREQ0, &chfreq0);
#ifdef MT7615
			MAC_IO_READ32(pAd, RMAC_CHFREQ1, &chfreq1);
#endif /* MT7615 */
			break;

		default:
			break;
		}

		ATEOp->SampleRssi(pAd, pRxBlk);

		/* RX packet counter calculate by chfreq of RXD */
		if (rxd_base->RxD1.ChFreq == chfreq0)
			ATECtrl->rx_stat.RxTotalCnt[TESTMODE_BAND0]++;

#ifdef MT7615
		else if (rxd_base->RxD1.ChFreq == chfreq1)
			ATECtrl->rx_stat.RxTotalCnt[TESTMODE_BAND1]++;

#endif /* MT7615 */
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Wrong chfreq!!\n""\tRXD.ch_freq: %x, chfreq0: %x, chfreq0: %x\n",
				__func__, rxd_base->RxD1.ChFreq, chfreq0, chfreq1));
	}
	/* LoopBack_Rx(pAd, pRxBlk->MPDUtotalByteCnt, pRxBlk->pRxPacket); */
	return TRUE;
}


INT8 mt_ate_release_wdev(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct wifi_dev *pWdev = NULL;
	UCHAR wdev_idx;

	wdev_idx = TESTMODE_GET_PARAM(ATECtrl, band_idx, wdev_idx);

	/* To close wdev */
	pWdev = pAd->wdev_list[wdev_idx];
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s: wdev_idx=%d\n", __func__, ATECtrl->wdev_idx));

	if (!pWdev)
		goto err;

	if (wdev_do_close(pWdev) != TRUE)
		ret = -1;

	/* Restore normal driver setting when release wdev */
	pWdev->channel = TESTMODE_GET_PARAM(ATECtrl, band_idx, backup_channel);
	pWdev->PhyMode = TESTMODE_GET_PARAM(ATECtrl, band_idx, backup_phymode);

	return ret;

err:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: Cannot get wdev by idx:%d\n", __func__, wdev_idx));

	return -1;
}


INT8 mt_ate_init_wdev(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct wifi_dev *pWdev = NULL;
	UCHAR wdev_idx;
	UCHAR channel;

	wdev_idx = TESTMODE_GET_PARAM(ATECtrl, band_idx, wdev_idx);
	channel = TESTMODE_GET_PARAM(ATECtrl, band_idx, Channel);

	/* To open wdev */
	pWdev = pAd->wdev_list[wdev_idx];
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: wdev_idx=%d, channel=%d\n",
		__func__, wdev_idx, channel));

	if (!pWdev)
		goto err;

	/* Backup normal driver setting when init wdev */
	TESTMODE_SET_PARAM(ATECtrl, band_idx, backup_channel, pWdev->channel);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, backup_phymode, pWdev->PhyMode);

	pWdev->channel = channel;

	if (band_idx == TESTMODE_BAND0)
		pWdev->PhyMode = PHYMODE_CAP_24G;
	else if (band_idx == TESTMODE_BAND1)
		pWdev->PhyMode = PHYMODE_CAP_5G;
	else
		pWdev->PhyMode = PHYMODE_CAP_24G;

	if (wdev_do_open(pWdev) != TRUE)
		ret = -1;
	if (wdev_edca_acquire(pAd, pWdev) != TRUE)
		ret = -1;

	HcSetEdca(pWdev);

	return ret;

err:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: Cannot get wdev by idx:%d\n", __func__, wdev_idx));

	return -1;
}


static INT32 MT_ATEReleaseBandInfo(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
#ifdef DBDC_MODE
	INT32 idx = band_idx - 1;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;

	if (ATECtrl->band_ext[idx].pate_pkt) {
		os_free_mem(ATECtrl->band_ext[idx].pate_pkt);
		ATECtrl->band_ext[idx].pate_pkt = NULL;
	}
#endif
	return ret;
}


static INT32 MT_ATEInitBandInfo(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
#ifdef DBDC_MODE
	INT32 idx = band_idx - 1;
	UCHAR *payload = NULL;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	RTMP_OS_COMPLETION *tx_wait = TESTMODE_GET_PADDR(ATECtrl, band_idx, tx_wait);
	CHAR addr[MAC_ADDR_LEN] = {0x00, 0x11, 0x22, 0xBA, 0x2D, 0x11};

	if (ATECtrl->band_ext[idx].pate_pkt)
		os_free_mem(ATECtrl->band_ext[idx].pate_pkt);

	ret = os_alloc_mem(pAd, (PUCHAR *)&ATECtrl->band_ext[idx].pate_pkt, ATE_TESTPKT_LEN);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, Channel, 36);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, ControlChl, 36);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, BW, 0);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, TxDoneCount, 0);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, TxedCount, 0);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, TxLength, 1024);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, pl_len, 1);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, QID, QID_AC_BE);

	payload = TESTMODE_GET_PARAM(ATECtrl, band_idx, payload);
	payload[0] = 0xAA;
	TESTMODE_SET_PARAM(ATECtrl, band_idx, HLen, LENGTH_802_11);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, FixedPayload, 1);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, TxCount, 0);
	ATECtrl->band_ext[idx].pkt_skb = NULL;
	NdisMoveMemory(ATECtrl->band_ext[idx].TemplateFrame, ATECtrl->TemplateFrame, 32);
	NdisMoveMemory(ATECtrl->band_ext[idx].Addr1, addr, MAC_ADDR_LEN);
	NdisMoveMemory(ATECtrl->band_ext[idx].Addr2, addr, MAC_ADDR_LEN);
	NdisMoveMemory(ATECtrl->band_ext[idx].Addr3, addr, MAC_ADDR_LEN);
	RTMP_OS_INIT_COMPLETION(tx_wait);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: idx:%u, pkt:%p\n",
		__func__, idx, ATECtrl->band_ext[idx].pate_pkt));
#endif
	return ret;
}


static INT32 MT_ATEStart(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
#if defined(MT7615) || defined(MT7622)
	struct _ATE_OPERATION    *ATEOp = ATECtrl->ATEOp;
#else
#endif /* defined(MT7615) || defined(MT7622) */
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	INT32 Ret = 0;
	BOOLEAN Cancelled;
	MT_RX_FILTER_CTRL_T rx_filter;
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
#endif
#ifdef DBDC_MODE
	struct _BAND_INFO *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */

	/* Remind FW that Enable ATE mode */
	MtCmdATEModeCtrl(pAd, 1);

#if (defined(MT_MAC) && (!defined(MT7636)))
#ifdef TXBF_SUPPORT
	/* Before going into ATE mode, stop sounding first */
	mt_Trigger_Sounding_Packet(pAd, FALSE, 0, 0, 0, NULL);
#endif /* TXBF_SUPPORT */
#endif /* MAC && undefined MT7636 */

	/* Make sure ATEInit successfully when interface up */
	if (ATECtrl->Mode & ATE_START) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: ATE has already started, wdev_idx:%u\n", __func__, ATECtrl->wdev_idx));
#ifdef DBDC_MODE
	if (IS_ATE_DBDC(pAd))
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: ATE has already started, wdev_idx:%u\n", __func__, Info->wdev_idx));
#endif /* DBDC_MODE */

		return Ret;
	}

	/* Allocate ATE TX packet buffer */
	if (!ATECtrl->pate_pkt)
		Ret = os_alloc_mem(pAd,	(PUCHAR *)&ATECtrl->pate_pkt, ATE_TESTPKT_LEN);

	if (Ret)
		goto err2;

	/* ATE data structure initialization */
	ATECtrl->Channel = 1;
	ATECtrl->ControlChl = 1;
	ATECtrl->BW = 0;
	ATECtrl->en_man_set_freq = 0;
	ATECtrl->TxDoneCount = 0;
	ATECtrl->TxedCount = 0;
	ATECtrl->TxLength = 1024;
	ATECtrl->pl_len = 1;
	ATECtrl->payload[0] = 0xAA;
	ATECtrl->QID = QID_AC_BE;
	ATECtrl->HLen = LENGTH_802_11;
	RTMP_OS_INIT_COMPLETION(&ATECtrl->tx_wait);

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
	mt_asic_pcie_aspm_dym_ctrl(pAd, DBDC_BAND0, FALSE, FALSE);
	if (pAd->CommonCfg.dbdc_mode)
		mt_asic_pcie_aspm_dym_ctrl(pAd, DBDC_BAND1, FALSE, FALSE);
	set_pcie_aspm_dym_ctrl_cap(pAd, FALSE);
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

	MT_ATEUpdateRxStatistic(pAd, TESTMODE_RESET_CNT, NULL);
	/*
	 *  Backup original CRs and change to ATE mode specific CR setting,
	 *  restore it back when back to normal mode
	 */
	mt_ate_mac_cr_backup_and_set(pAd);

	/* Common Part */
	/* ATECtrl->en_log = fATE_LOG_TXDUMP; */
	ATECtrl->en_log = 0;
	ATECtrl->verify_mode = HQA_VERIFY;
	ATECtrl->cmd_expire = RTMPMsecsToJiffies(3000);
	RTMP_OS_INIT_COMPLETION(&ATECtrl->cmd_done);
	ATECtrl->TxPower0 = pAd->EEPROMImage[TX0_G_BAND_TARGET_PWR];
	ATECtrl->TxPower1 = pAd->EEPROMImage[TX1_G_BAND_TARGET_PWR];
	NdisZeroMemory(ATECtrl->log_dump, sizeof(ATECtrl->log_dump[0])*ATE_LOG_TYPE_NUM);
	MT_ATEMPSInit(pAd);
	NdisZeroMemory(ATECtrl->pfmu_info, sizeof(ATECtrl->pfmu_info[0])*ATE_BFMU_NUM);
#ifdef CONFIG_QA
	MtAsicGetRxStat(pAd, HQA_RX_RESET_PHY_COUNT);
	MtAsicGetRxStat(pAd, HQA_RX_RESET_MAC_COUNT);
#endif
#ifdef ATE_TXTHREAD
	Ret = TESTMODE_TXTHREAD_INIT(pAd, 0);

	if (Ret)
		goto err3;
#endif /* ATE_TXTHREAD */

	MtATESetMacTxRx(pAd, ASIC_MAC_RX, FALSE, TESTMODE_BAND0);

	if (IS_ATE_DBDC(pAd))
		MtATESetMacTxRx(pAd, ASIC_MAC_RX, FALSE, TESTMODE_BAND1);

	/* Rx filter */
	os_zero_mem(&rx_filter, sizeof(rx_filter));
	rx_filter.bPromiscuous = FALSE;
	rx_filter.bFrameReport = TRUE;
	rx_filter.filterMask = RX_NDPA | RX_NOT_OWN_BTIM |
				RX_NOT_OWN_UCAST |
				RX_RTS | RX_CTS | RX_CTRL_RSV |
				RX_BC_MC_DIFF_BSSID_A2 |
				RX_BC_MC_DIFF_BSSID_A3 | RX_BC_MC_OWN_MAC_A3 |
				RX_PROTOCOL_VERSION |
				RX_FCS_ERROR;
	rx_filter.u1BandIdx = TESTMODE_BAND0;
	Ret = MtATESetRxFilter(pAd, rx_filter);

	if (IS_ATE_DBDC(pAd)) {
		rx_filter.u1BandIdx = TESTMODE_BAND1;
		Ret = MtATESetRxFilter(pAd, rx_filter);
	}

	/* Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID;
			 IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	/* Disable TX PDMA */
	AsicSetWPDMA(pAd, PDMA_TX_RX, FALSE);
	Ret = if_ops->init(pAd);
	Ret += if_ops->clean_trx_q(pAd);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/* first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev) {
				/* WifiSysApLinkUp(pAd, &pAd->ApCfg.MBSSID[IdBss].wdev); */
				pAd->ApCfg.MBSSID[IdBss].wdev.protection = 0;
			}
		}
	}
#endif
	AsicSetWPDMA(pAd, PDMA_TX_RX, TRUE);
#ifdef CONFIG_AP_SUPPORT
	APStop(pAd, pMbss, AP_BSS_OPER_ALL);
#endif /* CONFIG_AP_SUPPORT */
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	if (Ret)
		goto err0;

	/* MtTestModeWTBL2Update(pAd, 0); */
	RTMPCancelTimer(&pAd->Mlme.PeriodicTimer, &Cancelled);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);

	/* call wdev_do_open must behind ATE status be set */
	ATECtrl->Mode = ATE_START;
	if (IS_ATE_DBDC(pAd))
		TESTMODE_SET_PARAM(ATECtrl, TESTMODE_BAND1, Mode, ATE_START);

	Ret = mt_ate_init_wdev(pAd, TESTMODE_BAND0);

	if (IS_ATE_DBDC(pAd)) {
		Ret = MT_ATEInitBandInfo(pAd, TESTMODE_BAND1);
		Ret += mt_ate_init_wdev(pAd, TESTMODE_BAND1);
	}

#if defined(MT7615) || defined(MT7622)
	/* Tx Power related Status Initialization
	    Disable TX power related behavior when enter test mode */
	ATECtrl->fgTxPowerSKUEn        = FALSE;
	ATECtrl->fgTxPowerPercentageEn = FALSE;
	ATECtrl->fgTxPowerBFBackoffEn  = FALSE;
	ATECtrl->PercentageLevel       = 100;
#ifdef DBDC_MODE
	if (IS_ATE_DBDC(pAd)) {
		Info->fgTxPowerSKUEn        = FALSE;
		Info->fgTxPowerPercentageEn = FALSE;
		Info->fgTxPowerBFBackoffEn  = FALSE;
		Info->PercentageLevel       = 100;
	}
#endif /* DBDC_MODE */

	/* Disable Tx Power related feature for ATE mode */
#ifdef CONFIG_HW_HAL_OFFLOAD
	/* SKU */
	ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_SINGLE_SKU, FALSE);
	/* Power Percentage */
	ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_POWER_PERCENTAGE, FALSE);
	/* Power Drop */
	ATEOp->SetPowerDropLevel(pAd, 100);
	/* BF Backoff */
	ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_BF_BACKOFF, FALSE);
#endif /* CONFIG_HW_HAL_OFFLOAD */
#else
#endif /* defined(MT7615) || defined(MT7622) */

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: wdev_idx:%u\n", __func__, ATECtrl->wdev_idx));
#ifdef DBDC_MODE
	if (IS_ATE_DBDC(pAd))
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: wdev_idx:%u\n", __func__, Info->wdev_idx));
#endif /* DBDC_MODE */

	return Ret;

#ifdef ATE_TXTHREAD
err3:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: initial value fail, ret:%d\n", __func__, Ret));
	ATECtrl->Mode = ATE_STOP;
	MT_ATEReleaseLogDump(pAd);
#endif /* ATE_TXTHREAD */
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): if init fail, ret:%d\n", __func__, Ret));
err2:
	MTWF_LOG(DBG_CAT_TEST,  DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate test packet fail at pakcet\n", __func__));

	return Ret;
}


static INT32 MT_ATEStop(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	INT32 Ret = 0;
	MT_RX_FILTER_CTRL_T rx_filter;
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss;
	INT32 MaxNumBss = pAd->ApCfg.BssidNum; /* TODO: Delete after merge MT_ATEStop with Windows Test Mode */
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
#endif
	PNDIS_PACKET *pkt_skb = NULL;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	if ((ATECtrl->Mode & ATE_STOP) || !(ATECtrl->Mode & ATE_START))
		goto err2;

	mt_ate_mac_cr_restore(pAd);

	if (ATECtrl->Mode & ATE_FFT) {
		struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

		Ret = ATEOp->SetFFTMode(pAd, 0);
		Ret += MtCmdRfTestSwitchMode(pAd, OPERATION_NORMAL_MODE, 0, RF_TEST_DEFAULT_RESP_LEN);
		/* For FW to switch back to normal mode stable time */
		mdelay(2000);

		if (Ret)
			goto err0;

		ATECtrl->Mode &= ~ATE_FFT;
	}

	if (ATECtrl->Mode & fATE_IN_RFTEST) {
		Ret += MtCmdRfTestSwitchMode(pAd, OPERATION_NORMAL_MODE, 0, RF_TEST_DEFAULT_RESP_LEN);
		/* For FW to switch back to normal mode stable time */
		mdelay(2000);

		if (Ret)
			goto err0;
	}

	/* MT76x6 Test Mode Freqency offset restore*/
	if (ATECtrl->en_man_set_freq) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Manual Set Frequency Restore\n"));
		MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_ENABLE);
		MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_VALUE);
		ATECtrl->en_man_set_freq = 0;
	}

	os_zero_mem(&rx_filter, sizeof(rx_filter));
	rx_filter.bPromiscuous = FALSE;
	rx_filter.bFrameReport = FALSE;
	rx_filter.filterMask = RX_NDPA | RX_NOT_OWN_BTIM |
				RX_NOT_OWN_UCAST |
				RX_RTS | RX_CTS | RX_CTRL_RSV |
				RX_BC_MC_DIFF_BSSID_A2 |
				RX_BC_MC_DIFF_BSSID_A3 | RX_BC_MC_OWN_MAC_A3 |
				RX_PROTOCOL_VERSION |
				RX_FCS_ERROR;
	rx_filter.u1BandIdx = 0;
	Ret = MtATESetRxFilter(pAd, rx_filter);

	if (IS_ATE_DBDC(pAd)) {
		rx_filter.u1BandIdx = 1;
		Ret = MtATESetRxFilter(pAd, rx_filter);
	}

	MT_ATEReleaseLogDump(pAd);
	MT_ATEMPSRelease(pAd);

	/* Release skb */
	pkt_skb = &ATECtrl->pkt_skb;

	if (*pkt_skb) {
		RELEASE_NDIS_PACKET(pAd, *pkt_skb, NDIS_STATUS_SUCCESS);
		*pkt_skb = NULL;
	}

#ifdef DBDC_MODE
	if (IS_ATE_DBDC(pAd)) {
		pkt_skb = &ATECtrl->band_ext[TESTMODE_BAND1 - 1].pkt_skb;

		if (*pkt_skb) {
			RELEASE_NDIS_PACKET(pAd, *pkt_skb, NDIS_STATUS_SUCCESS);
			*pkt_skb = NULL;
		}
	}
#endif /*DBDC_MODE */
#ifdef ATE_TXTHREAD
	Ret = TESTMODE_TXTHREAD_RELEASE(pAd, 0);
	msleep(2);
#endif /* ATE_TXTHREAD */

	if (if_ops->clean_trx_q)
		Ret = if_ops->clean_trx_q(pAd);

	if (if_ops->ate_leave)
		Ret += if_ops->ate_leave(pAd);

	if (Ret)
		goto err1;

	Ret = mt_ate_release_wdev(pAd, TESTMODE_BAND0);

	if (IS_ATE_DBDC(pAd)) {
		Ret = mt_ate_release_wdev(pAd, TESTMODE_BAND1);
		Ret += MT_ATEReleaseBandInfo(pAd, TESTMODE_BAND1);
	}

	NICInitializeAdapter(pAd);

	/* RTMPEnableRxTx(pAd); */
	if (pAd->CommonCfg.bTXRX_RXV_ON) {
		MtATESetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, TRUE, TESTMODE_BAND0);

		if (IS_ATE_DBDC(pAd))
			MtATESetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, TRUE, TESTMODE_BAND1);
	} else {
		/* Normal mode enabled RXV and interface down up will crash if disalbe it */
		MtATESetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, TRUE, TESTMODE_BAND0);

		if (IS_ATE_DBDC(pAd))
			MtATESetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, TRUE, TESTMODE_BAND1);
	}

#ifdef ARBITRARY_CCK_OFDM_TX
	if (IS_MT7615(pAd)) {
		MtATEInitCCK_OFDM_Path(pAd, TESTMODE_BAND0);

		if (IS_ATE_DBDC(pAd))
		MtATEInitCCK_OFDM_Path(pAd, TESTMODE_BAND1);
	}
#endif
	RTMP_OS_EXIT_COMPLETION(&ATECtrl->cmd_done);
	ATECtrl->Mode = ATE_STOP;
	TESTMODE_SET_PARAM(ATECtrl, TESTMODE_BAND1, Mode, ATE_STOP);
	MtCmdATEModeCtrl(pAd, 0); /* Remind FW that Disable ATE mode */
	RTMPSetTimer(&pAd->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);

#ifdef CONFIG_AP_SUPPORT
	APStartUp(pAd, pMbss, AP_BSS_OPER_ALL);
#endif /* CONFIG_AP_SUPPROT  */
	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
	if (MaxNumBss > MAX_MBSSID_NUM(pAd))
		MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID;
			IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(
					pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif

	if (Ret)
		goto err1;

	return Ret;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF-test stop fail, ret:%d\n", __func__, Ret));
err1:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: if stop fail, ret:%d\n", __func__, Ret));
err2:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ATE has already stopped ret:%d\n", __func__, Ret));

	ATECtrl->Mode = ATE_STOP;
	return Ret;
}
#else
static INT32 MT_ATEStart(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret = 0;
#ifdef CAL_FREE_IC_SUPPORT
	BOOLEAN bCalFree = 0;
#endif /* CAL_FREE_IC_SUPPORT */
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: 2\n", __func__));
#ifdef CAL_FREE_IC_SUPPORT
	RTMP_CAL_FREE_IC_CHECK(pAd, bCalFree);

	if (bCalFree) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Cal Free IC!!\n"));
		RTMP_CAL_FREE_DATA_GET(pAd);
	} else
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Non Cal Free IC!!\n"));

#endif /* CAL_FREE_IC_SUPPORT */

	/* MT7636 Test Mode Freqency offset restore*/
	if (ATECtrl->en_man_set_freq) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MT76x6 Manual Set Frequency Restore\n"));
		MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_ENABLE);
		MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_VALUE);
		ATECtrl->en_man_set_freq = 0;
	}

	if (ATECtrl->Mode & fATE_TXCONT_ENABLE) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Stop Continuous Tx\n", __func__));
		ATEOp->StopContinousTx(pAd, 0);
	}

	if (ATECtrl->Mode & fATE_TXCARRSUPP_ENABLE) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Stop Carrier Suppression Test\n", __func__));
		ATEOp->StopTxTone(pAd);
	}

	/* Reset ATE TX/RX Counter */
	ATECtrl->TxLength = 1024;
	ATECtrl->QID = QID_AC_BE;
	ATECtrl->TxPower0 = 0x10;
	ATECtrl->TxPower1 = 0x10;
	ATECtrl->en_man_set_freq = 0;
	ATECtrl->TxDoneCount = 0;
	ATECtrl->Mode = ATE_START;
	MtAsicGetRxStat(pAd, HQA_RX_RESET_PHY_COUNT);
	MtAsicGetRxStat(pAd, HQA_RX_RESET_MAC_COUNT);
	/* MtTestModeWTBL2Update(pAd, 0); */
	return Ret;
}


static INT32 MT_ATEStop(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	MtATESetMacTxRx(pAd, ASIC_MAC_RXV, FALSE, TESTMODE_BAND0);

	/* RTMPEnableRxTx(pAd); */

	/* MT7636 Test Mode Freqency offset restore*/
	if (ATECtrl->en_man_set_freq) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MT76x6 Manual Set Frequency Restore\n"));
		MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_ENABLE);
		MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_VALUE);
		ATECtrl->en_man_set_freq = 0;
	}

	ATECtrl->Mode = ATE_STOP;
	return Ret;
}
#endif


INT mt_ate_set_tmac_info(RTMP_ADAPTER *pAd, TMAC_INFO *tmac_info, UINT32 band_idx)
{
	INT32 Ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_TX_TIME_PARAM *tx_time_param = TESTMODE_GET_PADDR(ATECtrl, band_idx, tx_time_param);
	UCHAR *addr1 = NULL;
	UCHAR phy_mode = 0;
	UCHAR mcs = 0;
	UCHAR vht_nss = 1;
	UINT32 ant_sel = 0;
	INT wdev_idx = 0;
	UINT32 pkt_tx_time = tx_time_param->pkt_tx_time;
	UINT8 need_qos = tx_time_param->pkt_need_qos;
	UINT8 need_amsdu = tx_time_param->pkt_need_amsdu;
	UINT8 need_ampdu = tx_time_param->pkt_need_ampdu;
#if !defined(COMPOS_TESTMODE_WIN)
	struct wifi_dev *pWdev = NULL;
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
	UCHAR WmmIdx;
#endif /* defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) */
	BOOLEAN fgSPE;

	wdev_idx = TESTMODE_GET_PARAM(ATECtrl, band_idx, wdev_idx);
	pWdev = pAd->wdev_list[wdev_idx];
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: wdev_idx=%d\n", __func__, wdev_idx));

	if (!pWdev)
		goto err0;

#endif
	TESTMODE_SET_PARAM(ATECtrl, band_idx, HLen, LENGTH_802_11);
	addr1 = TESTMODE_GET_PARAM(ATECtrl, band_idx, Addr1);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:: addr1: %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, PRINT_MAC(addr1)));
	ant_sel = TESTMODE_GET_PARAM(ATECtrl, band_idx, TxAntennaSel);
	phy_mode = TESTMODE_GET_PARAM(ATECtrl, band_idx, PhyMode);
	mcs = TESTMODE_GET_PARAM(ATECtrl, band_idx, Mcs);
	vht_nss = TESTMODE_GET_PARAM(ATECtrl, band_idx, Nss);
	/* Fill TMAC_INFO */
	NdisZeroMemory(tmac_info, sizeof(*tmac_info));
	tmac_info->LongFmt = TRUE;

	if (pkt_tx_time > 0) {
		tmac_info->WifiHdrLen = (UINT8) tx_time_param->pkt_hdr_len;
		tmac_info->PktLen = (UINT16) tx_time_param->pkt_msdu_len;
		tmac_info->NeedTrans = FALSE;

		if (need_qos | need_amsdu | need_ampdu) {
			tmac_info->HdrPad = 2;
			tmac_info->BmcPkt = FALSE;
			tmac_info->UserPriority = 0;
		} else {
			tmac_info->HdrPad = 0;
			tmac_info->BmcPkt = IS_BM_MAC_ADDR(addr1);
			tmac_info->UserPriority = 0;
		}
	} else {
		tmac_info->WifiHdrLen = (UINT8)TESTMODE_GET_PARAM(ATECtrl, band_idx, HLen);
		tmac_info->HdrPad = 0;
		tmac_info->PktLen = (UINT16)TESTMODE_GET_PARAM(ATECtrl, band_idx, TxLength);
		tmac_info->BmcPkt = IS_BM_MAC_ADDR(addr1);
	}

	/* no ack */
	if ((pkt_tx_time > 0) && (need_ampdu))
		tmac_info->bAckRequired = 1;
	else
		tmac_info->bAckRequired = 0;

#if !defined(COMPOS_TESTMODE_WIN)
	tmac_info->FrmType = FC_TYPE_DATA;
	tmac_info->SubType = SUBTYPE_QDATA;
	tmac_info->OwnMacIdx = pWdev->OmacIdx;
#else
	tmac_info->OwnMacIdx = 0;
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: tmac_info->OwnMacIdx=%d\n", __func__, tmac_info->OwnMacIdx));
	/* no frag */
	tmac_info->FragIdx = 0;
	/* no protection */
	tmac_info->CipherAlg = 0;
	/* TX Path setting */
	tmac_info->VhtNss = vht_nss ? vht_nss : 1;
	tmac_info->AntPri = 0;
	tmac_info->SpeEn = 0;

	/* Timing Measure setting */
	if ((pAd->pTmrCtrlStruct != NULL) && (pAd->pTmrCtrlStruct->TmrEnable == TMR_INITIATOR))
		tmac_info->TimingMeasure = 1;

	/* band_idx for TX ring choose */
	tmac_info->band_idx = band_idx;

	switch (ant_sel) {
	case 0: /* Both */
		tmac_info->AntPri = 0;
		tmac_info->SpeEn = 1;
		break;

	case 1: /* TX0 */
		tmac_info->AntPri = 0;
		tmac_info->SpeEn = 0;
		break;

	case 2: /* TX1 */
		tmac_info->AntPri = 2; /* b'010 */
		tmac_info->SpeEn = 0;
		break;
	}

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
	/* Need to modify the way of wmm_idx getting */
	WmmIdx = TESTMODE_GET_PARAM(ATECtrl, band_idx, wmm_idx);
	tmac_info->WmmSet = WmmIdx;

	if (IS_MT7615(pAd) || IS_MT7622(pAd) ||	IS_P18(pAd) || IS_MT7663(pAd)) {
		if (ant_sel & ATE_ANT_USER_SEL) {
			ant_sel &= ~ATE_ANT_USER_SEL;
			tmac_info->AntPri = ant_sel;
		} else {
			INT map_idx = 0;

			for (map_idx = 0; map_idx < ARRAY_SIZE(ant_to_spe_idx_map); map_idx++) {
				if (ant_sel == ant_to_spe_idx_map[map_idx].ant_sel)
					break;
			}

			if (map_idx == ARRAY_SIZE(ant_to_spe_idx_map))
				tmac_info->AntPri = 0;
			else
				tmac_info->AntPri = ant_to_spe_idx_map[map_idx].spe_idx;
		}

#if defined(TXBF_SUPPORT) && defined(MT_MAC)

		if (g_EBF_certification) {
			if (BF_ON_certification) {
				tmac_info->AntPri = 0;
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("tmac_info->AntPri = 0\n"));
			} else {
				tmac_info->AntPri = 24;  /* 24 */
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("tmac_info->AntPri = 24\n"));
			}
		}

#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:: ant_sel:%x, ant_pri:%u, vht_nss:%x, TxD.VhtNss:%x\n",
			__func__, ant_sel, tmac_info->AntPri, vht_nss, tmac_info->VhtNss));
	}

#endif /* defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) */
	/* Fill transmit setting */
	tmac_info->TxRadioSet.RateCode = mcs;
	tmac_info->TxRadioSet.CurrentPerPktBW = TESTMODE_GET_PARAM(ATECtrl, band_idx, PerPktBW);
	tmac_info->TxRadioSet.ShortGI =	TESTMODE_GET_PARAM(ATECtrl, band_idx, Sgi);
	tmac_info->TxRadioSet.Stbc = TESTMODE_GET_PARAM(ATECtrl, band_idx, Stbc);
	tmac_info->TxRadioSet.PhyMode = phy_mode;
	tmac_info->TxRadioSet.Ldpc = TESTMODE_GET_PARAM(ATECtrl, band_idx, Ldpc);

	tmac_info->QueIdx = dmac_wmm_swq_2_hw_ac_que[tmac_info->WmmSet][TESTMODE_GET_PARAM(ATECtrl, band_idx, QID)];

	if ((pkt_tx_time > 0) && (need_ampdu)) {
		tmac_info->Wcid = ATECtrl->wcid_ref;
		tmac_info->FixRate = 0;
		tmac_info->BaDisable = FALSE;
		tmac_info->RemainTxCnt = 1;
	} else {
		tmac_info->Wcid = 0;
		tmac_info->FixRate = 1;
		tmac_info->BaDisable = TRUE;
		tmac_info->RemainTxCnt = 15;
	}

	if (ATECtrl->txs_enable) {
		tmac_info->TxS2Host = TRUE;
		tmac_info->TxS2Mcu = FALSE;
		tmac_info->TxSFmt = 1;
	}

	if (phy_mode == MODE_CCK) {
		tmac_info->TxRadioSet.Premable = LONG_PREAMBLE;

		if (mcs == MCS_9) {
			tmac_info->TxRadioSet.RateCode = 0;
			tmac_info->TxRadioSet.Premable = SHORT_PREAMBLE;
		} else if (mcs == MCS_10) {
			tmac_info->TxRadioSet.RateCode = 1;
			tmac_info->TxRadioSet.Premable = SHORT_PREAMBLE;
		} else if (mcs == MCS_11) {
			tmac_info->TxRadioSet.RateCode = 2;
			tmac_info->TxRadioSet.Premable = SHORT_PREAMBLE;
		}
	}

#ifdef TXBF_SUPPORT
	else {
		UCHAR iTxBf = TESTMODE_GET_PARAM(ATECtrl, band_idx, iTxBf);
		UCHAR eTxBf = TESTMODE_GET_PARAM(ATECtrl, band_idx, eTxBf);

		if (iTxBf || eTxBf) {
			tmac_info->TxRadioSet.ItxBFEnable = iTxBf;
			tmac_info->TxRadioSet.EtxBFEnable = eTxBf;
		}
	}

#endif /* TXBF_SUPPORT */
	tmac_info->Wcid = ATECtrl->wcid_ref;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: tmac_info->TxRadioSet.EtxBFEnable=%d, tmac_info->Wcid=%d\n",
		__func__, tmac_info->TxRadioSet.EtxBFEnable, tmac_info->Wcid));

	if (tmac_info->AntPri >= 24)
		fgSPE = TRUE;
	else
		fgSPE = FALSE;

#ifdef SINGLE_SKU_V2
	/* Update Power offset according to Band, Phymode, MCS, BW, Nss, SPE */
	tmac_info->PowerOffset = SKUTxPwrOffsetGet(pAd,
							band_idx, ATECtrl->PerPktBW, ATECtrl->PhyMode,
							ATECtrl->Mcs,
							ATECtrl->Nss, fgSPE);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: tmac_info->PowerOffset = 0x%x (%d)\n",
		__func__, tmac_info->PowerOffset, tmac_info->PowerOffset));

	if (tmac_info->PowerOffset < -16)
		tmac_info->PowerOffset = -16;
	else if (tmac_info->PowerOffset > 15)
		tmac_info->PowerOffset = 15;
#endif /* SINGLE_SKU_V2 */

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
	if ((pkt_tx_time > 0) && (need_ampdu)) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: tmac_info->Wcid/Wmmset/QueIdx=%d/%d/%d\n",
			__func__, tmac_info->Wcid, tmac_info->WmmSet, tmac_info->QueIdx));
	}
#endif /* defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) */
	return Ret;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:: Cannot get Wdev by idx:%d\n", __func__, wdev_idx));
	return -1;
}


UINT32 mt_ate_get_txlen_by_pkt_tx_time(struct _ATE_CTRL *ATECtrl, UINT32 band_idx)
{
	struct _ATE_TX_TIME_PARAM *tx_time_param = TESTMODE_GET_PADDR(ATECtrl, band_idx, tx_time_param);
	UINT32 txlen = 0;
	UINT32 hlen = 0;
	UINT32 tx_data_rate = 0;
	UINT32 pkt_tx_time = 0;
	UCHAR phy_mode = 0;
	UCHAR mcs = 0, mcs_1ss, nss = 1;
	UCHAR bw = 0, bw_fact = 1;
	UCHAR sgi = 0;

	/*
	 * 1. Get the tx data rate
	 * 2. Get the packet tx time
	 * 3. Calculate the packet length by tx_data_rate and packet_tx_time
	 * 4. Return txlen
	 */
	pkt_tx_time = tx_time_param->pkt_tx_time;
	hlen = TESTMODE_GET_PARAM(ATECtrl, band_idx, HLen);
	phy_mode = TESTMODE_GET_PARAM(ATECtrl, band_idx, PhyMode);
	mcs = TESTMODE_GET_PARAM(ATECtrl, band_idx, Mcs);
	bw = TESTMODE_GET_PARAM(ATECtrl, band_idx, BW);
	sgi = TESTMODE_GET_PARAM(ATECtrl, band_idx, Sgi);
	mcs_1ss = mcs;

	if (phy_mode == MODE_CCK) { /* Legacy CCK mode */
		UINT8 cck_map_idx = 0;

		for (cck_map_idx = 0;
			 cck_map_idx < ARRAY_SIZE(cck_mode_mcs_to_data_rate_map); cck_map_idx++) {
			if (mcs_1ss == cck_mode_mcs_to_data_rate_map[cck_map_idx].mcs)
				break;
		}

		if (cck_map_idx == ARRAY_SIZE(cck_mode_mcs_to_data_rate_map)) {
			tx_data_rate = cck_mode_mcs_to_data_rate_map[0].tx_data_rate;
			mcs = mcs_1ss =	cck_mode_mcs_to_data_rate_map[0].mcs;
			TESTMODE_SET_PARAM(ATECtrl, band_idx, Mcs, mcs);
		} else
			tx_data_rate = cck_mode_mcs_to_data_rate_map[cck_map_idx].tx_data_rate;

		/* Transfer from bit to byte with expected tx time */
		txlen = pkt_tx_time * tx_data_rate / 1000 / 8;
	} else if (phy_mode == MODE_OFDM) { /* Legacy OFDM mode */
		UINT8 ofdm_map_idx = 0;

		for (ofdm_map_idx = 0; ofdm_map_idx < ARRAY_SIZE(ofdm_mode_mcs_to_data_rate_map); ofdm_map_idx++) {
			if (mcs_1ss == ofdm_mode_mcs_to_data_rate_map[ofdm_map_idx].mcs)
				break;
		}

		if (ofdm_map_idx == ARRAY_SIZE(ofdm_mode_mcs_to_data_rate_map)) {
			tx_data_rate = ofdm_mode_mcs_to_data_rate_map[0].tx_data_rate;
			mcs = mcs_1ss =	ofdm_mode_mcs_to_data_rate_map[0].mcs;
			TESTMODE_SET_PARAM(ATECtrl, band_idx, Mcs, mcs);
		} else
			tx_data_rate = ofdm_mode_mcs_to_data_rate_map[ofdm_map_idx].tx_data_rate;

		/* Transfer from bit to byte with expected tx time */
		txlen = pkt_tx_time * tx_data_rate / 1000 / 8;
	} else if (phy_mode == MODE_HTMIX || phy_mode == MODE_HTGREENFIELD) { /* HT mode */
		UINT8 n_map_idx = 0;

		if (mcs != MCS_32) {
			mcs_1ss = mcs % 8;
			nss = (mcs / 8) + 1;
			bw_fact = (bw == BW_40) ? 2 : 1;
		} else {
			bw_fact = 1;
			nss = 1;
		}

		for (n_map_idx = 0; n_map_idx < ARRAY_SIZE(n_mode_mcs_to_data_rate_map); n_map_idx++) {
			if (mcs_1ss == n_mode_mcs_to_data_rate_map[n_map_idx].mcs)
				break;
		}

		if (n_map_idx == ARRAY_SIZE(n_mode_mcs_to_data_rate_map)) {
			tx_data_rate = n_mode_mcs_to_data_rate_map[0].tx_data_rate;
			mcs = mcs_1ss =	n_mode_mcs_to_data_rate_map[0].mcs;
			TESTMODE_SET_PARAM(ATECtrl, band_idx, Mcs, mcs);
		} else
			tx_data_rate = n_mode_mcs_to_data_rate_map[n_map_idx].tx_data_rate;

		tx_data_rate = tx_data_rate * nss * bw_fact;

		if (sgi == 1)
			tx_data_rate = (tx_data_rate / 9) * 10;

		/* Transfer from bit to byte with expected tx time */
		txlen = pkt_tx_time * tx_data_rate / 1000 / 8;
	} else if (phy_mode == MODE_VHT) { /* VHT mode */
		UINT8 ac_map_idx = 0;
		struct _ATE_DATA_RATE_MAP *vht_rate_map;
		UINT32 array_cnt = 0;

		if (bw == BW_20) {
			vht_rate_map = ac_mode_mcs_to_data_rate_map_bw20;
			array_cnt = ARRAY_SIZE(ac_mode_mcs_to_data_rate_map_bw20);
		} else if (bw == BW_40) {
			vht_rate_map = ac_mode_mcs_to_data_rate_map_bw40;
			array_cnt = ARRAY_SIZE(ac_mode_mcs_to_data_rate_map_bw40);
		} else if (bw == BW_80) {
			vht_rate_map = ac_mode_mcs_to_data_rate_map_bw80;
			array_cnt = ARRAY_SIZE(ac_mode_mcs_to_data_rate_map_bw80);
		} else if (bw == BW_160) {
			vht_rate_map = ac_mode_mcs_to_data_rate_map_bw160;
			array_cnt = ARRAY_SIZE(ac_mode_mcs_to_data_rate_map_bw160);
		} else {
			vht_rate_map = ac_mode_mcs_to_data_rate_map_bw20;
			array_cnt = ARRAY_SIZE(ac_mode_mcs_to_data_rate_map_bw20);
		}

		for (ac_map_idx = 0; ac_map_idx < array_cnt; ac_map_idx++) {
			if (mcs == vht_rate_map[ac_map_idx].mcs)
				break;
		}

		if (ac_map_idx == array_cnt) {
			tx_data_rate = vht_rate_map[0].tx_data_rate;
			mcs = mcs_1ss = vht_rate_map[0].mcs;
			TESTMODE_SET_PARAM(ATECtrl, band_idx, Mcs, mcs);
		} else
			tx_data_rate = vht_rate_map[ac_map_idx].tx_data_rate;

		/* TODO: Need to check for SGI equation! */
		if (sgi == 1)
			tx_data_rate = (tx_data_rate / 9) * 10;

		/* Transfer from bit to byte with expected tx time */
		txlen = pkt_tx_time * tx_data_rate / 10 / 8;
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: phy_mode=%d, mcs/mcs_1ss=%d/%d, nss=%d, bw/bw_fact=%d/%d, sgi=%d\n",
		__func__, phy_mode, mcs, mcs_1ss, nss, bw, bw_fact, sgi));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: txlen=%d, pkt_tx_time=%d, tx_data_rate=%d\n",
		__func__, txlen, pkt_tx_time, tx_data_rate));

	if (phy_mode == MODE_VHT) {
		if (txlen >= (MAX_VHT_MPDU_LEN * 64)) {
			txlen = (MAX_VHT_MPDU_LEN * 64);
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: Expected txlen > VHT mode PPDU max length, reduce the txlen=%d\n",
				__func__, txlen));
		}
	} else if (phy_mode == MODE_HTMIX || phy_mode == MODE_HTGREENFIELD) {
		if (txlen >= MAX_HT_AMPDU_LEN) {
			txlen = MAX_HT_AMPDU_LEN;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: Expected txlen > HT mode PPDU max length, reduce the txlen=%d\n", __func__, txlen));
		}
	} else if (phy_mode == MODE_OFDM) {
		if (txlen >= MAX_MSDU_LEN) {
			txlen = MAX_MSDU_LEN;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: Expected txlen > OFDM mode PPDU max length, reduce the txlen=%d\n", __func__, txlen));
		}
	} else if (phy_mode == MODE_CCK) {
		if (txlen >= MAX_MSDU_LEN) {
			txlen = MAX_MSDU_LEN;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: Expected txlen > CCK mode PPDU max length, reduce the txlen=%d\n", __func__, txlen));
		}
	}

	return txlen;
}


UINT32 mt_ate_get_hlen_by_pkt_tx_time(
	struct _ATE_CTRL *ATECtrl,
	UINT32 band_idx,
	UINT32 txlen,
	BOOLEAN *need_qos,
	BOOLEAN *need_amsdu,
	BOOLEAN *need_ampdu)
{
	UINT32 pkt_len = 0;
	UINT32 hlen = 0;
	UCHAR phy_mode = 0;
	UCHAR use_data_frame = 1;
	/*
	 * 1. Get the tx data rate
	 * 2. Check if need to send packet with AMPDU format
	 * 3. Check if need to send packet with AMSDU-in-AMPDU format
	 * 4. Return the expected packet header length by tx packet type
	 *  if need to has QoS field and HTC field.
	*/
	phy_mode = TESTMODE_GET_PARAM(ATECtrl, band_idx, PhyMode);
	pkt_len = txlen;

	if (pkt_len <= MIN_MSDU_LEN) {
		use_data_frame = 0;
		/* Here we need to go mgmt/ctrl frame mode */
	} else if (pkt_len <= MAX_MSDU_LEN) {
		if (phy_mode == MODE_VHT)
			*need_qos = 1;
	} else if (pkt_len <= DEFAULT_MPDU_LEN) {
		if (phy_mode == MODE_HTMIX || phy_mode == MODE_HTGREENFIELD) {
			*need_amsdu = 1;
			*need_qos = 1;
		} else if (phy_mode == MODE_VHT)
			*need_qos = 1;
		else {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Can't generate frame with such length for CCK/OFDM mode!!\n", __func__));
		}
	} else if (pkt_len <= MAX_VHT_MPDU_LEN) {
		if (phy_mode == MODE_VHT)
			*need_qos = 1;
		else if (phy_mode == MODE_HTMIX || phy_mode == MODE_HTGREENFIELD) {
			*need_ampdu = 1;
			*need_amsdu = 1;
			*need_qos = 1;
		} else {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Can't generate frame with such length for CCK/OFDM mode!!\n", __func__));
		}
	} else {
		if ((phy_mode == MODE_VHT) || (phy_mode == MODE_HTMIX || phy_mode == MODE_HTGREENFIELD)) {
			*need_ampdu = 1;
			*need_amsdu = 1;
			*need_qos = 1;
		} else {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Can't generate frame with such length for CCK/OFDM mode!!\n", __func__));
		}
	}

	hlen = TESTMODE_GET_PARAM(ATECtrl, band_idx, HLen);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: original header len=%d\n", __func__, hlen));

	if (use_data_frame) {
		hlen = DEFAULT_MAC_HDR_LEN;

		if (*need_qos)
			hlen = QOS_MAC_HDR_LEN;
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: pkt_len=%d, need_qos/amsdu/ampdu/dataframe/hlen=%d/%d/%d/%d/%d\n",
			  __func__, pkt_len, *need_qos, *need_amsdu, *need_ampdu, use_data_frame, hlen));
	return hlen;
}


INT mt_ate_wtbl_cfg(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT ret;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_TX_TIME_PARAM *tx_time_param = TESTMODE_GET_PADDR(ATECtrl, band_idx, tx_time_param);
	UINT8 need_qos, need_amsdu, need_ampdu;
	UCHAR phy_mode, mcs, nss, bw, sgi, stbc;
	struct cmd_wtbl_dw_mask_set dw_set[7];
	UINT32 wtbl_rate = 0, ant_sel = 0;
	need_qos = tx_time_param->pkt_need_qos;
	need_amsdu = tx_time_param->pkt_need_amsdu;
	need_ampdu = tx_time_param->pkt_need_ampdu;
	phy_mode = TESTMODE_GET_PARAM(ATECtrl, band_idx, PhyMode);
	mcs = TESTMODE_GET_PARAM(ATECtrl, band_idx, Mcs);
	nss = TESTMODE_GET_PARAM(ATECtrl, band_idx, Nss);
	bw = TESTMODE_GET_PARAM(ATECtrl, band_idx, BW);
	sgi = TESTMODE_GET_PARAM(ATECtrl, band_idx, Sgi);
	stbc = TESTMODE_GET_PARAM(ATECtrl, band_idx, Stbc);
	ant_sel = TESTMODE_GET_PARAM(ATECtrl, band_idx,	TxAntennaSel);
	NdisZeroMemory((UCHAR *)&dw_set[0],	sizeof(dw_set));
	/* Decide TxCap, HT/VHT/LPDC (DW2) */
	dw_set[0].ucWhichDW = 2;
	dw_set[0].u4DwMask = 0x9FFFFFFF;

	switch (phy_mode) {
	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		dw_set[0].u4DwValue = (0x1 << 29);
		break;

	case MODE_VHT:
		dw_set[0].u4DwValue = (0x3 << 29);
		break;

	default:
		dw_set[0].u4DwValue = 0;
		break;
	};

	/* Decide AF/I_PSM (DW3) */
	dw_set[1].ucWhichDW = 3;

	dw_set[1].u4DwMask = 0xD8E0F000;

	if (need_ampdu) {
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)

		if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_P18(pAd) || IS_MT7663(pAd)) {
			if (ant_sel & ATE_ANT_USER_SEL)
				ant_sel &= ~ATE_ANT_USER_SEL;
			else {
				INT map_idx = 0;

				for (map_idx = 0; map_idx < ARRAY_SIZE(ant_to_spe_idx_map); map_idx++) {
					if (ant_sel == ant_to_spe_idx_map[map_idx].ant_sel)
						break;
				}

				if (map_idx == ARRAY_SIZE(ant_to_spe_idx_map))
					ant_sel = 0;
				else
					ant_sel = ant_to_spe_idx_map[map_idx].spe_idx;
			}
		}

#endif /* defined(MT7615) || defined(MT7622) */
		dw_set[1].u4DwValue = (0x1 << 29)
					| (0x7 << 24)
					| ((ant_sel & 0x1F) << 16)
					| ((ant_sel & 0x7) << 9)
					| ((ant_sel & 0x7) << 6)
					| ((ant_sel & 0x7) << 3)
					| ((ant_sel & 0x7) << 0);
	} else
		dw_set[1].u4DwValue = 0;

	/* Decide BA-enable/BA-winsize/BA-bitmap (DW4) */
	dw_set[2].ucWhichDW = 4;
	dw_set[2].u4DwMask = 0x0;

	if (need_ampdu)
		dw_set[2].u4DwValue = 0xFFFFFFFF;
	else
		dw_set[2].u4DwValue = 0x0;

	/* Decide FCAP/G2/G4/G8/G16/QoS-enable (DW5 )*/
	dw_set[3].ucWhichDW = 5;
	dw_set[3].u4DwMask = 0xFFF7C0FF;

	switch (bw) {
	case BW_20:
		dw_set[3].u4DwValue = (bw << 12) | (sgi << 8);
		break;

	case BW_40:
		dw_set[3].u4DwValue = (bw << 12) | (sgi << 9);
		break;

	case BW_80:
		dw_set[3].u4DwValue = (bw << 12) | (sgi << 10);
		break;

	case BW_160:
		dw_set[3].u4DwValue = (bw << 12) | (sgi << 11);
		break;

	default:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Can't find such bw, use default\n", __func__));
		dw_set[3].u4DwValue = (BW_20 << 12) | (sgi << 8);
		break;
	}

	if (phy_mode == MODE_HTMIX || phy_mode == MODE_HTGREENFIELD || phy_mode == MODE_VHT) {
		/* QoS enable by phymode */
		dw_set[3].u4DwValue |= (1 << 19);
	}

	/* Use phymode/mcs/nss/STBC decide RateInfo (DW6~8) */
	/* step 1. depends on ATE command tx rate, convert to 12-bits WTBL-rate value */
	/* wtbl_rate = ((STBC & 0x1) << 11) | ((Nss & 0x3)<< 9) | ((phy_mode & 0x3)  << 6) |  ((mcs & 0x3f) << 0) */
	if (phy_mode == MODE_CCK)
		wtbl_rate = tx_rate_to_tmi_rate(phy_mode, mcs, (nss + 1), stbc, 1);
	else
		wtbl_rate = tx_rate_to_tmi_rate(phy_mode, mcs, (nss + 1), stbc, 0);

	/* step 2. set WTBL RAW DW 6: ((rate3 & 0xff)<< 24) | ((rate2 & 0xfff) << 12) | ((rate1 & 0xfff) << 0) */
	dw_set[4].ucWhichDW = 6;
	dw_set[4].u4DwMask = 0x0;
	dw_set[4].u4DwValue = ((wtbl_rate & 0xFF) << 24)
				| ((wtbl_rate & 0xFFF) << 12)
				| ((wtbl_rate & 0xFFF) << 0);
	dw_set[5].ucWhichDW = 7;
	dw_set[5].u4DwMask = 0x0;
	dw_set[5].u4DwValue = ((wtbl_rate & 0xF) << 28)
				| ((wtbl_rate & 0xFFF) << 16)
				| ((wtbl_rate & 0xFFF) << 4)
				| (((wtbl_rate & 0xF00) >> 8) << 0);
	dw_set[6].ucWhichDW = 8;
	dw_set[6].u4DwMask = 0x0;
	dw_set[6].u4DwValue = ((wtbl_rate & 0xFFF) << 20)
				| ((wtbl_rate & 0xFFF) << 8)
				| (((wtbl_rate & 0xFF0) >> 4) << 0);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: phy_mode=%d, mcs=%d, nss=%d, stbc=%d, wtbl_rate=0x%x\n",
			  __func__, phy_mode, mcs, nss, stbc, wtbl_rate));
	ret = WtblResetAndDWsSet(pAd, ATECtrl->wcid_ref, 1, sizeof(dw_set) / sizeof(struct cmd_wtbl_dw_mask_set), dw_set);
	return ret;
}


INT32 mt_ate_ampdu_frame(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_TX_TIME_PARAM *tx_time_param = TESTMODE_GET_PADDR(ATECtrl, band_idx, tx_time_param);
	UCHAR *addr1, *addr2, *addr3;
	UCHAR phy_mode;
	UCHAR *tmac_info, *pheader, *payload, *frm_template;
	UINT32 txlen, hlen;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->TXWISize;
	UINT32 new_txlen, new_hlen;
	UINT8 need_qos, need_amsdu, need_ampdu;
	UINT32 per_mpdu_len = 0;
	UINT8 ampdu_agg_cnt = 0;
	phy_mode = TESTMODE_GET_PARAM(ATECtrl, band_idx, PhyMode);
	new_txlen = tx_time_param->pkt_tx_len;
	new_hlen = tx_time_param->pkt_hdr_len;
	need_qos = tx_time_param->pkt_need_qos;
	need_amsdu = tx_time_param->pkt_need_amsdu;
	need_ampdu = tx_time_param->pkt_need_ampdu;

	if (phy_mode == MODE_VHT)
		per_mpdu_len = (MAX_VHT_MPDU_LEN - 100); /* include mac header length */
	else
		per_mpdu_len = (DEFAULT_MPDU_LEN - 100); /* include mac header length */

	tx_time_param->pkt_msdu_len = per_mpdu_len;
	ampdu_agg_cnt = new_txlen / per_mpdu_len;

	if (new_txlen % per_mpdu_len)
		ampdu_agg_cnt++;

	tx_time_param->pkt_ampdu_cnt = ampdu_agg_cnt;

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
	ret = mt_ate_ampdu_ba_limit(pAd, band_idx, ampdu_agg_cnt);
	ret = mt_ate_set_sta_pause_cr(pAd, band_idx);
#endif /* defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) */
	addr1 = TESTMODE_GET_PARAM(ATECtrl, band_idx, Addr1);
	addr2 = TESTMODE_GET_PARAM(ATECtrl, band_idx, Addr2);
	addr3 = TESTMODE_GET_PARAM(ATECtrl, band_idx, Addr3);
	txlen = tx_time_param->pkt_msdu_len;
	hlen = tx_time_param->pkt_hdr_len;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s(wcid:%d, txlen/hlen/buf=%d/%d/%p, pkt_tx_len/pkt_msdu_len/pkt_ampdu_cnt=%d/%d/%d)\n"
			  "\tDA: %02x:%02x:%02x:%02x:%02x:%02x\n"
			  "\tSA: %02x:%02x:%02x:%02x:%02x:%02x\n"
			  "\tBSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",
			  __func__, ATECtrl->wcid_ref, txlen, hlen, buf,
			  tx_time_param->pkt_tx_len, tx_time_param->pkt_msdu_len,
			  tx_time_param->pkt_ampdu_cnt, PRINT_MAC(addr1), PRINT_MAC(addr2), PRINT_MAC(addr3)));
	tmac_info = buf;
	pheader = (buf + tx_hw_hdr_len);
	payload = (pheader + hlen);
	NdisZeroMemory(buf, ATE_TESTPKT_LEN);
#ifdef TXBF_SUPPORT
	{
		UCHAR iTxBf = TESTMODE_GET_PARAM(ATECtrl, band_idx, iTxBf);
		UCHAR eTxBf = TESTMODE_GET_PARAM(ATECtrl, band_idx, eTxBf);

		/* Use wcid 1~4 */
		if (iTxBf || eTxBf) {
			if ((ATECtrl->wcid_ref > ATE_BFMU_NUM) || (ATECtrl->wcid_ref < 1))
				ATECtrl->wcid_ref = ATE_BF_WCID;

			addr1 = ATECtrl->pfmu_info[ATECtrl->wcid_ref - 1].addr;
		}

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s:: DA: %02x:%02x:%02x:%02x:%02x:%02x, wcid=%u\n",
				  __func__, PRINT_MAC(addr1), ATECtrl->wcid_ref));
	}
#endif
	frm_template = TESTMODE_GET_PARAM(ATECtrl, band_idx, TemplateFrame);
	NdisMoveMemory(pheader, frm_template, hlen);
	NdisMoveMemory(pheader + 4, addr1, MAC_ADDR_LEN);
	NdisMoveMemory(pheader + 10, addr2, MAC_ADDR_LEN);
	NdisMoveMemory(pheader + 16, addr3, MAC_ADDR_LEN);
	ret = MtATEPayloadInit(pAd, payload, txlen - hlen, band_idx);
#if !defined(COMPOS_TESTMODE_WIN)

	if (ATECtrl->en_log & fATE_LOG_TXDUMP) {
		INT i = 0;
		PHEADER_802_11 hdr = (HEADER_802_11 *)pheader;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[TXCONTENT DUMP START]\n"));
		dump_tmac_info(pAd, tmac_info);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[TXD RAW]: "));

		for (i = 0; i < tx_hw_hdr_len; i++)
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%04x", tmac_info[i]));

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\nADDR1: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr1)));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("ADDR2: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr2)));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("ADDR3: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr3)));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("FC: %04x\n", *(UINT16 *)(&hdr->FC)));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tFrom DS: %x\n", hdr->FC.FrDs));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tTo DS: %x\n", hdr->FC.ToDs));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("[CONTENT RAW]: "));

		for (i = 0; i < (txlen - hlen); i++)
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%02x", payload[i]));

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n[TXCONTENT DUMP END]\n"));
	}

#endif /* !defined(COMPOS_TESTMODE_WIN) */
	TESTMODE_SET_PARAM(ATECtrl, band_idx, is_alloc_skb, 0);
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pheader, DIR_WRITE, FALSE);
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		MTMacInfoEndianChange(pAd, tmac_info, TYPE_TMACINFO, sizeof(TMAC_TXD_L));

#endif
#endif
	return ret;
}


INT32 mt_ate_non_ampdu_frame(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_TX_TIME_PARAM *tx_time_param = TESTMODE_GET_PADDR(ATECtrl, band_idx, tx_time_param);
	UCHAR *tmac_info, *pheader, *payload, *addr1, *addr2, *addr3, *frm_template;
	UINT32 txlen, hlen;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->TXWISize;
	addr1 = TESTMODE_GET_PARAM(ATECtrl, band_idx, Addr1);
	addr2 = TESTMODE_GET_PARAM(ATECtrl, band_idx, Addr2);
	addr3 = TESTMODE_GET_PARAM(ATECtrl, band_idx, Addr3);
	txlen = tx_time_param->pkt_msdu_len;
	hlen = tx_time_param->pkt_hdr_len;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s(wcid:%d, txlen/hlen/buf=%d/%d/%p, pkt_tx_len/pkt_msdu_len=%d/%d)\n"
			  "\tDA: %02x:%02x:%02x:%02x:%02x:%02x\n"
			  "\tSA: %02x:%02x:%02x:%02x:%02x:%02x\n"
			  "\tBSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",
			  __func__, ATECtrl->wcid_ref,
			  txlen, hlen, buf,
			  tx_time_param->pkt_tx_len,
			  tx_time_param->pkt_msdu_len,
			  PRINT_MAC(addr1), PRINT_MAC(addr2),
			  PRINT_MAC(addr3)));
	tmac_info = buf;
	pheader = (buf + tx_hw_hdr_len);
	payload = (pheader + hlen);
	NdisZeroMemory(buf, ATE_TESTPKT_LEN);
#ifdef TXBF_SUPPORT
	{
		UCHAR iTxBf = TESTMODE_GET_PARAM(ATECtrl, band_idx, iTxBf);
		UCHAR eTxBf = TESTMODE_GET_PARAM(ATECtrl, band_idx, eTxBf);

		/* Use wcid 1~4 */
		if (iTxBf || eTxBf) {
			if ((ATECtrl->wcid_ref > ATE_BFMU_NUM) || (ATECtrl->wcid_ref < 1))
				ATECtrl->wcid_ref = ATE_BF_WCID;

			addr1 = ATECtrl->pfmu_info[ATECtrl->wcid_ref - 1].addr;
		}

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s:: DA: %02x:%02x:%02x:%02x:%02x:%02x, wcid=%u\n",
				  __func__, PRINT_MAC(addr1), ATECtrl->wcid_ref));
	}
#endif
	frm_template = TESTMODE_GET_PARAM(ATECtrl, band_idx, TemplateFrame);
	NdisMoveMemory(pheader, frm_template, hlen);
	NdisMoveMemory(pheader + 4, addr1, MAC_ADDR_LEN);
	NdisMoveMemory(pheader + 10, addr2, MAC_ADDR_LEN);
	NdisMoveMemory(pheader + 16, addr3, MAC_ADDR_LEN);
	ret = MtATEPayloadInit(pAd, payload, txlen - hlen, band_idx);
#if !defined(COMPOS_TESTMODE_WIN)

	if (ATECtrl->en_log & fATE_LOG_TXDUMP) {
		INT i = 0;
		PHEADER_802_11 hdr = (HEADER_802_11 *)pheader;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("[TXCONTENT DUMP START]\n"));
		dump_tmac_info(pAd, tmac_info);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("[TXD RAW]: "));

		for (i = 0; i < tx_hw_hdr_len; i++)
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%04x", tmac_info[i]));

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\nADDR1: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr1)));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("ADDR2: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr2)));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("ADDR3: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr3)));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("FC: %04x\n", *(UINT16 *)(&hdr->FC)));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tFrom DS: %x\n", hdr->FC.FrDs));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tTo DS: %x\n", hdr->FC.ToDs));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("[CONTENT RAW]: "));

		for (i = 0; i < (txlen - hlen); i++)
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%02x", payload[i]));

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n[TXCONTENT DUMP END]\n"));
	}

#endif /* !defined(COMPOS_TESTMODE_WIN) */
	TESTMODE_SET_PARAM(ATECtrl, band_idx, is_alloc_skb, 0);
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pheader, DIR_WRITE, FALSE);
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		MTMacInfoEndianChange(pAd, tmac_info, TYPE_TMACINFO, sizeof(TMAC_TXD_L));

#endif
#endif
	return ret;
}


INT32 MT_ATEGenBurstPkt(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_TX_TIME_PARAM *tx_time_param = TESTMODE_GET_PADDR(ATECtrl, band_idx, tx_time_param);
	UINT32 new_txlen, new_hlen;
	UINT8 need_qos, need_amsdu, need_ampdu;
	new_txlen = tx_time_param->pkt_tx_len;
	new_hlen = tx_time_param->pkt_hdr_len;
	need_qos = tx_time_param->pkt_need_qos;
	need_amsdu = tx_time_param->pkt_need_amsdu;
	need_ampdu = tx_time_param->pkt_need_ampdu;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: new_txlen/new_hlen=%d/%d, qos/amsdu/ampdu=%d/%d/%d\n",
			  __func__, new_txlen, new_hlen, need_qos, need_amsdu, need_ampdu));

	/* Update WTBL if necessary */
	if (need_ampdu)
		mt_ate_wtbl_cfg(pAd, band_idx);

	/* Generate tx packet */
	if (need_ampdu)
		ret = mt_ate_ampdu_frame(pAd, buf, band_idx);
	else {
		/* No aggregation, directly go with specific length and through ALTX queue */
		tx_time_param->pkt_ampdu_cnt = 1;
		tx_time_param->pkt_msdu_len = new_txlen;
		ret = mt_ate_non_ampdu_frame(pAd, buf, band_idx);
	}

	return ret;
}


INT32 MT_ATEGenPkt(RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_TX_TIME_PARAM *tx_time_param = TESTMODE_GET_PADDR(ATECtrl, band_idx, tx_time_param);
	UCHAR *tmac_info, *pheader, *payload, *addr1, *addr2, *addr3, *template;
	UINT32 txlen, hlen, pkt_tx_time;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->TXWISize;

	if (!buf) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: NULL buf, band_idx:%u\n", __func__, band_idx));
		return NDIS_STATUS_FAILURE;
	}

	/* For long packet implemetation */
	tx_time_param->pkt_tx_len = 0;
	tx_time_param->pkt_hdr_len = 0;
	tx_time_param->pkt_need_qos = 0;
	tx_time_param->pkt_need_amsdu = 0;
	tx_time_param->pkt_need_ampdu = 0;
	tx_time_param->pkt_ampdu_cnt = 0;
	pkt_tx_time = tx_time_param->pkt_tx_time;

	if (pkt_tx_time > 0) {
		UINT8 need_qos = 0, need_amsdu = 0, need_ampdu = 0;
		UINT32 new_txlen = mt_ate_get_txlen_by_pkt_tx_time(ATECtrl, band_idx);
		UINT32 new_hlen;
		txlen = TESTMODE_GET_PARAM(ATECtrl, band_idx, TxLength);
		hlen = TESTMODE_GET_PARAM(ATECtrl, band_idx, HLen);

		if (new_txlen > 0)
			txlen = new_txlen;
		else {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Can't get txlen by pkt tx time\n", __func__));
		}

		new_hlen = mt_ate_get_hlen_by_pkt_tx_time(ATECtrl, band_idx, txlen, &need_qos, &need_amsdu, &need_ampdu);

		if (new_hlen > 0)
			hlen = new_hlen;
		else {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Can't get hdrlen by pkt tx time\n", __func__));
		}

		tx_time_param->pkt_tx_len = new_txlen;
		tx_time_param->pkt_hdr_len = hlen;
		tx_time_param->pkt_need_qos = need_qos;
		tx_time_param->pkt_need_amsdu = need_amsdu;
		tx_time_param->pkt_need_ampdu = need_ampdu;
		/* New packet generation function */
		ret = MT_ATEGenBurstPkt(pAd, buf, band_idx);
		return ret;
	}

	addr1 = TESTMODE_GET_PARAM(ATECtrl, band_idx, Addr1);
	addr2 = TESTMODE_GET_PARAM(ATECtrl, band_idx, Addr2);
	addr3 = TESTMODE_GET_PARAM(ATECtrl, band_idx, Addr3);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s(wcid:%d)::\tDA: %02x:%02x:%02x:%02x:%02x:%02x\n\tSA: %02x:%02x:%02x:%02x:%02x:%02x\n\tBSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",
			  __func__, ATECtrl->wcid_ref, PRINT_MAC(addr1), PRINT_MAC(addr2), PRINT_MAC(addr3)));
	template = TESTMODE_GET_PARAM(ATECtrl, band_idx, TemplateFrame);
	txlen = TESTMODE_GET_PARAM(ATECtrl, band_idx, TxLength);
	hlen = TESTMODE_GET_PARAM(ATECtrl, band_idx, HLen);

	/* Error check for txlen */
	if (txlen == 0) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: TX length can't be 0!!\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	tmac_info = buf;
	pheader = (buf + tx_hw_hdr_len);
	payload = (pheader + hlen);
	NdisZeroMemory(buf, ATE_TESTPKT_LEN);
#ifdef TXBF_SUPPORT
	{
		UCHAR iTxBf = TESTMODE_GET_PARAM(ATECtrl, band_idx, iTxBf);
		UCHAR eTxBf = TESTMODE_GET_PARAM(ATECtrl, band_idx, eTxBf);

		/* Use wcid 1~4 */
		if (iTxBf || eTxBf) {
			if ((ATECtrl->wcid_ref > ATE_BFMU_NUM) || (ATECtrl->wcid_ref < 1))
				ATECtrl->wcid_ref = ATE_BF_WCID;

			addr1 = ATECtrl->pfmu_info[ATECtrl->wcid_ref - 1].addr;
		}

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s:: DA: %02x:%02x:%02x:%02x:%02x:%02x, wcid=%u\n", __func__, PRINT_MAC(addr1), ATECtrl->wcid_ref));
	}
#endif
	NdisMoveMemory(pheader, template, hlen);
	NdisMoveMemory(pheader + 4, addr1, MAC_ADDR_LEN);
	NdisMoveMemory(pheader + 10, addr2, MAC_ADDR_LEN);
	NdisMoveMemory(pheader + 16, addr3, MAC_ADDR_LEN);
	ret = MtATEPayloadInit(pAd, payload, txlen - hlen, band_idx);
#if !defined(COMPOS_TESTMODE_WIN)

	if (ATECtrl->en_log & fATE_LOG_TXDUMP) {
		INT i = 0;
		PHEADER_802_11 hdr = (HEADER_802_11 *)pheader;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[TXCONTENT DUMP START]\n"));
		dump_tmac_info(pAd, tmac_info);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[TXD RAW]: "));

		for (i = 0; i < tx_hw_hdr_len; i++)
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%04x", tmac_info[i]));

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\nADDR1: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr1)));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("ADDR2: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr2)));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("ADDR3: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(hdr->Addr3)));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("FC: %04x\n", *(UINT16 *)(&hdr->FC)));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tFrom DS: %x\n", hdr->FC.FrDs));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\tTo DS: %x\n", hdr->FC.ToDs));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("[CONTENT RAW]: "));

		for (i = 0; i < (txlen - hlen); i++)
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%02x", payload[i]));

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n[TXCONTENT DUMP END]\n"));
	}

#endif /* !defined(COMPOS_TESTMODE_WIN) */
	TESTMODE_SET_PARAM(ATECtrl, band_idx, is_alloc_skb, 0);
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pheader, DIR_WRITE, FALSE);
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		MTMacInfoEndianChange(pAd, tmac_info, TYPE_TMACINFO, sizeof(TMAC_TXD_L));

#endif
#endif
	return ret;
}


UINT32 mt_ate_mcs32_handle(RTMP_ADAPTER *pAd, UINT8 wcid_ref, UINT8 bw)
{
	INT32 ret = 0;
	UINT32 DwMask = 0;
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)

	if (IS_MT7615(pAd) || IS_MT7622(pAd) ||	IS_P18(pAd) || IS_MT7663(pAd)) {
		union WTBL_DW5 wtbl_txcap;
		DwMask = ~(3 <<	12); /* only update fcap bit[13:12] */
		wtbl_txcap.field.fcap = bw;
		/* WTBLDW5 */
		WtblDwSet(pAd, wcid_ref, 1, 5, DwMask, wtbl_txcap.word);
		return ret;
	}

#else
	{
		union WTBL_2_DW9 wtbl_txcap;
		DwMask = ~(3 <<	14); /* only update fcap bit[15:14] */
		wtbl_txcap.field.fcap = bw;
		/* WTBL2DW9 */
		WtblDwSet(pAd, wcid_ref, 2, 9, DwMask, wtbl_txcap.word);
	}
#endif
	return ret;
}


#if !defined(COMPOS_TESTMODE_WIN)	/* 1todo too many OS private functio */
INT32 MT_ATETxPkt(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_TX_TIME_PARAM *tx_time_param = TESTMODE_GET_PADDR(ATECtrl, band_idx, tx_time_param);
	PNDIS_PACKET pkt;
	INT32 ret = 0;
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen;
	PNDIS_PACKET *pkt_skb = NULL;
	UCHAR *pate_pkt = TESTMODE_GET_PARAM(ATECtrl, band_idx, pate_pkt);
	UINT32 txlen = TESTMODE_GET_PARAM(ATECtrl, band_idx, TxLength);
	UINT32 tx_cnt = TESTMODE_GET_PARAM(ATECtrl, band_idx, TxCount);
	UINT32 txdone_cnt = TESTMODE_GET_PARAM(ATECtrl,	band_idx, TxDoneCount);
	USHORT qid =  TESTMODE_GET_PARAM(ATECtrl, band_idx, QID);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->TXWISize;
	UCHAR wdev_idx = TESTMODE_GET_PARAM(ATECtrl, band_idx, wdev_idx);
	struct wifi_dev *wdev = wdev_search_by_idx(pAd, wdev_idx);
	UINT32 pkt_tx_time = tx_time_param->pkt_tx_time;
	UINT8 need_ampdu = tx_time_param->pkt_need_ampdu;
	struct sk_buff *skb = NULL;
	struct sk_buff *skb2 = NULL;
	int alloced;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s: band_idx:%u, tx_cnt:%u, txdone_cnt:%u, pate_pkt:%p, ring Idx:%u\n",
		__func__, band_idx, tx_cnt, txdone_cnt, pate_pkt, qid));

	if (!pate_pkt) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: invalid pate_pkt(%p)\n", __func__, pate_pkt));
		goto done;
	}

	if (band_idx == 0)
		pkt_skb = &ATECtrl->pkt_skb;

#ifdef DBDC_MODE
	else if (IS_ATE_DBDC(pAd) && band_idx == 1)
		pkt_skb = &ATECtrl->band_ext[band_idx -	1].pkt_skb;
#endif /* DBDC_MODE */
	alloced = TESTMODE_GET_PARAM(ATECtrl, band_idx,	is_alloc_skb);

	if (!alloced) {
		if (*pkt_skb)
			RELEASE_NDIS_PACKET(pAd, *pkt_skb, NDIS_STATUS_SUCCESS);

		if (pkt_tx_time > 0)
			txlen = tx_time_param->pkt_msdu_len;

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s: Alloc pkt, txlen=%d, tx_hw_hdr_len=%d, total=%d\n", __func__, txlen, tx_hw_hdr_len, txlen + tx_hw_hdr_len));
		ret = RTMPAllocateNdisPacket(pAd, pkt_skb, NULL, 0, pate_pkt, txlen + tx_hw_hdr_len);

		if (ret != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: AllocateNdisPacket fail\n", __func__));
			goto done;
		}

		TESTMODE_SET_PARAM(ATECtrl, band_idx, is_alloc_skb, 1);
	}

	skb = (struct sk_buff *)(*pkt_skb);
	OS_PKT_CLONE(pAd, skb, skb2, GFP_ATOMIC);

	if (skb2 == NULL) {
		RELEASE_NDIS_PACKET(pAd, *pkt_skb, NDIS_STATUS_SUCCESS);
		pkt_skb = NULL;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: clone pakcet fail\n", __func__));
		goto done;
	}

	pkt = (PNDIS_PACKET)skb2;
	RTMP_SET_PACKET_WDEV(pkt, wdev_idx);

	if (pkt_tx_time > 0) {
		UINT8 wcid = ATECtrl->wcid_ref;
		RTMP_SET_PACKET_WCID(pkt, wcid);
	}

	RTMP_QueryPacketInfo(pkt, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:: allocate pkt, wdev_idx=%d, q_idx:%x, pkt_va:%p, VA:%p, Len:%u\n",
			  __func__, wdev_idx, qid, pkt, pSrcBufVA, SrcBufLen));
	/* Set the packet type to ATE frame before enqueue packet */
	RTMP_SET_PACKET_TXTYPE(pkt, TX_ATE_FRAME);

	if ((pkt_tx_time > 0) && (need_ampdu)) {
		RTMP_SET_PACKET_TYPE(pkt, TX_DATA);
		RTMP_SET_PACKET_QUEIDX(pkt, QID_AC_BE);
	} else {
		RTMP_SET_PACKET_TYPE(pkt, TX_MGMT);
		RTMP_SET_PACKET_QUEIDX(pkt, 0);
	}

	ret = send_mlme_pkt(pAd, pkt, wdev, qid, FALSE);
done:
	return ret;
}


static UINT32 MT_ATEGetBandIdx(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt)
{
	UINT32 band_idx = 0;
	UCHAR wdev_idx = 0;
	struct wifi_dev *wdev = NULL;

	wdev_idx = RTMP_GET_PACKET_WDEV(pkt);

	if (wdev_idx >= WDEV_NUM_MAX)
		goto err0;

	wdev = pAd->wdev_list[wdev_idx];

	if (!wdev)
		goto err0;

	band_idx = HcGetBandByChannel(pAd, wdev->channel);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s: wdev_idx:%x\n", __func__, wdev_idx));
	return band_idx;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: cannot find wdev by idx:%x\n", __func__, wdev_idx));
	return -1;
}


INT32 MT_ATETxControl(RTMP_ADAPTER *pAd, UINT32 band_idx, PNDIS_PACKET pkt)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UINT32 txdone_cnt = 0, tx_cnt = 0, mode = 0;

	if (ATECtrl->verify_mode == ATE_LOOPBACK) {
		if (pAd->LbCtrl.LoopBackWaitRx) {
#ifdef RTMP_PCI_SUPPORT

			if (IS_PCI_INF(pAd))
				RTMP_OS_COMPLETE(&pAd->LbCtrl.LoopBackPCITxEvent);
			else {
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("Not supported in this interface yet\n"));
			}

#else
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Not supported in this interface yet\n"));
#endif
		}
	} else if (ATECtrl->verify_mode == HQA_VERIFY) {
		/* Need to get band_idx first if free token done */
		if (pkt)
			band_idx = MT_ATEGetBandIdx(pAd, pkt);

		if (band_idx == -1) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Wrong band_idx %u\n", __func__, band_idx));
			goto done;
		}

		txdone_cnt = TESTMODE_GET_PARAM(ATECtrl, band_idx, TxDoneCount);
		tx_cnt = TESTMODE_GET_PARAM(ATECtrl, band_idx, TxCount);
		mode = TESTMODE_GET_PARAM(ATECtrl, band_idx, Mode);

		/* Do not count in packet number when TX is not in start stage */
		if (!(mode & ATE_TXFRAME))
			return ret;

		/* Triggered when RX tasklet free token */
		if (pkt) {
			pAd->RalinkCounters.KickTxCount++;
			txdone_cnt++;
			TESTMODE_SET_PARAM(ATECtrl, band_idx, TxDoneCount, txdone_cnt);
		}

#ifdef ATE_TXTHREAD
		TESTMODEThreadProceedTx(pAd, band_idx);
#else

		if ((mode & ATE_TXFRAME) && (txdone_cnt < tx_cnt))
			ret = MT_ATETxPkt(pAd, band_idx);
		else if ((mode & ATE_TXFRAME) &&
				 (txdone_cnt == tx_cnt)) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("All Tx is done\n"));

			if (mode & fATE_MPS) {
				RTMP_OS_COMPLETION *tx_wait = TESTMODE_GET_PADDR(ATECtrl, band_idx, tx_wait);
				RTMP_OS_COMPLETE(tx_wait);
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Finish one MPS Item\n"));
			}

			/* Tx status enters idle mode */
			TESTMODE_SET_PARAM(ATECtrl, band_idx, TxStatus, 0);
		} else if (!(mode & ATE_TXFRAME)) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Stop TX bottom is pressed\n"));

			if (mode & fATE_MPS) {
				RTMP_OS_COMPLETION *tx_wait = TESTMODE_GET_PADDR(ATECtrl, band_idx, tx_wait);
				mode &= ~fATE_MPS;
				TESTMODE_SET_PARAM(ATECtrl, band_idx, Mode, mode);
				RTMP_OS_COMPLETE(tx_wait);
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MPS STOP\n"));
			}
		} else {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 ("ATE:: DO NOT match any condition, Mode:0x%x, TxCnt:%u, TxDone:%u\n", mode, tx_cnt, txdone_cnt));
		}

		TESTMODE_SET_PARAM(ATECtrl, band_idx, TxDoneCount, txdone_cnt);
#endif
	}

done:
	return ret;
}


static INT32 mt_ate_calculate_duty_cycle(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	struct _ATE_IPG_PARAM *ipg_param = TESTMODE_GET_PADDR(ATECtrl, band_idx, ipg_param);
	struct _ATE_TX_TIME_PARAM *tx_time_param = TESTMODE_GET_PADDR(ATECtrl, band_idx, tx_time_param);
	UINT32 ipg, pkt_tx_time, duty_cycle;
	ipg = ipg_param->ipg;
	pkt_tx_time = tx_time_param->pkt_tx_time;
	duty_cycle = TESTMODE_GET_PARAM(ATECtrl, band_idx, duty_cycle);

	/* Calculate needed ipg/pkt_tx_time and duty_cycle */
	if ((duty_cycle > 0) && (pkt_tx_time == 0) && (ipg == 0)) {
		/* TODO: need to consider this case in the future */
		duty_cycle = 0;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: There are no pkt_tx_time/ipg!!\n"
				  "%s: Use default transmission setting and set duty_cycle=%d\n",
				  __func__, __func__, duty_cycle));
	} else if ((duty_cycle > 0) && (pkt_tx_time > 0) && (ipg == 0)) {
		ipg = ((pkt_tx_time * 100) / duty_cycle) - pkt_tx_time;
		ipg_param->ipg = ipg;
		/* If IPG value is not make sense, there's error handle when get ipg parameter */
		ret = ATEOp->SetIPG(pAd);
	} else if ((duty_cycle > 0) && (pkt_tx_time == 0) && (ipg > 0)) {
		/* If pkt_tx_time is not make sense, there's error handle when start TX */
		pkt_tx_time = (duty_cycle * ipg) / (100 - duty_cycle);
	} else {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Already existed pkt_tx_time/ipg, can't set duty_cycle!!\n"
				  "%s: Expected duty_cycle=%d%%\n", __func__, __func__, duty_cycle));
		duty_cycle = (pkt_tx_time * 100) / (pkt_tx_time + ipg);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Real duty_cycle=%d%%\n", __func__, duty_cycle));
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: duty_cycle=%d%%, ipg=%dus, pkt_tx_time=%dus\n",
			  __func__, duty_cycle, ipg, pkt_tx_time));
	tx_time_param->pkt_tx_time = pkt_tx_time;
	TESTMODE_SET_PARAM(ATECtrl, band_idx, duty_cycle, duty_cycle);
	return ret;
}


static UINT16 mt_ate_set_ifs_cr(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IPG_PARAM *ipg_param = TESTMODE_GET_PADDR(ATECtrl, band_idx, ipg_param);
	UINT16 slot_time, sifs_time;
	UINT32 txv_time = 0, i2t_chk_time = 0, tr2t_chk_time = 0;
	UINT32 value = 0;
	slot_time = ipg_param->slot_time;
	sifs_time = ipg_param->sifs_time;
	/* in uint of ns */
	MAC_IO_READ32(pAd, TMAC_ATCR, &txv_time);
	txv_time *= NORMAL_CLOCK_TIME;
	i2t_chk_time = (UINT32)(slot_time * 1000) - txv_time - BBP_PROCESSING_TIME;
	tr2t_chk_time = (UINT32)(sifs_time * 1000) - txv_time - BBP_PROCESSING_TIME;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: txv_time=%dns, i2t_chk_time=%dns, tr2t_chk_time=%dns\n",
		__func__, txv_time, i2t_chk_time, tr2t_chk_time));
	i2t_chk_time /= NORMAL_CLOCK_TIME;
	tr2t_chk_time /= NORMAL_CLOCK_TIME;

	if (band_idx == TESTMODE_BAND0) {
		MAC_IO_READ32(pAd, TMAC_TRCR0, &value);
		value = (value & 0xFE00FE00)
				| ((i2t_chk_time & 0x1FF) << 16)
				| ((tr2t_chk_time & 0x1FF) << 0);
		MAC_IO_WRITE32(pAd, TMAC_TRCR0, value);
	} else if (band_idx == TESTMODE_BAND1) {
		MAC_IO_READ32(pAd, TMAC_TRCR1, &value);
		value = (value & 0xFE00FE00)
				| ((i2t_chk_time & 0x1FF) << 16)
				| ((tr2t_chk_time & 0x1FF) << 0);
		MAC_IO_WRITE32(pAd, TMAC_TRCR1, value);
	} else {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Invalid band_idx!!\n", __func__));
		return FALSE;
	}

	return ret;
}


static INT32 mt_ate_set_wmm_param_by_qid(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IPG_PARAM *ipg_param = TESTMODE_GET_PADDR(ATECtrl, band_idx, ipg_param);
	INT wdev_idx = 0;
	UCHAR WmmIdx = 0xFF;
	USHORT qid;
	UINT16 slot_time, sifs_time, cw;
	UINT8 ac_num, aifsn;
#if !defined(COMPOS_TESTMODE_WIN)
	struct wifi_dev *pWdev = NULL;
	wdev_idx = TESTMODE_GET_PARAM(ATECtrl, band_idx, wdev_idx);
	pWdev = pAd->wdev_list[wdev_idx];

	if (pWdev) {
		WmmIdx = TESTMODE_GET_PARAM(ATECtrl, band_idx, wmm_idx);

		if (WmmIdx > 3) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: wdev_idx=%d, invalid WmmIdx=%d, reset to 0!\n",  __func__, wdev_idx, WmmIdx));
			WmmIdx = 0xFF;
		}
	} else {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Invalid WDEV, reset WmmIdx to 0!\n", __func__));
		WmmIdx = 0xFF;
	}

#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: wdev_idx=%d, WmmIdx=%d\n", __func__, wdev_idx, WmmIdx));
	qid = TESTMODE_GET_PARAM(ATECtrl, band_idx, QID);

	if ((qid != QID_AC_BE) && (qid != TxQ_IDX_ALTX0) && (qid != TxQ_IDX_ALTX1)) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Impossible!\n", __func__));
		return FALSE;
	}

	slot_time = ipg_param->slot_time;
	sifs_time = ipg_param->sifs_time;
	ac_num = qid;
	aifsn = ipg_param->aifsn;
	cw = ipg_param->cw;
	ret = MtAsicSetWmmParam(pAd, WmmIdx, (UINT32)ac_num, WMM_PARAM_AIFSN, (UINT32)aifsn);
	ret = MtAsicSetWmmParam(pAd, WmmIdx, (UINT32)ac_num, WMM_PARAM_CWMIN, (UINT32)cw);
	ret = MtAsicSetWmmParam(pAd, WmmIdx, (UINT32)ac_num, WMM_PARAM_CWMAX, (UINT32)cw);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: qid=%d, slot_time=%d, sifs_time=%d, ac_num=%d, aifsn=%d, cw=%d\n",
			  __func__, qid, slot_time, sifs_time, ac_num, aifsn, cw));
	return ret;
}


static INT32 mt_ate_apply_ipg_param(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IPG_PARAM *ipg_param = TESTMODE_GET_PADDR(ATECtrl, band_idx, ipg_param);
	UINT32 ipg;
	UINT16 slot_time, sifs_time, cw;
	UINT8 aifsn;
	ipg = ipg_param->ipg;

	if (ipg > 0) {
		/* Get packet qIdx and decide which CR set need to be changed */
		slot_time = ipg_param->slot_time;
		sifs_time = ipg_param->sifs_time;
		aifsn = ipg_param->aifsn;
		cw = ipg_param->cw;
	} else {
		/* Write default value back to HW */
		slot_time = DEFAULT_SLOT_TIME;
		sifs_time = DEFAULT_SIFS_TIME;
		aifsn = MIN_AIFSN;
		cw = 0;
	}

	ipg_param->slot_time = slot_time;
	ipg_param->sifs_time = sifs_time;
	ipg_param->aifsn = aifsn;
	ipg_param->cw = cw;
	ret = mt_ate_set_ifs_cr(pAd, band_idx);
#ifdef CONFIG_HW_HAL_OFFLOAD
	ret = MtCmdATESetSlotTime(pAd, (UINT8)slot_time, (UINT8)sifs_time, RIFS_TIME, EIFS_TIME, (UINT8)band_idx);
#endif
	ret = mt_ate_set_wmm_param_by_qid(pAd, band_idx);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: ipg=%d, slot_time=%d, sifs_time=%d, aifsn=%d, cw=%d\n",
			  __func__, ipg, slot_time, sifs_time, aifsn, cw));
	return ret;
}

BOOLEAN mt_ate_fill_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	PACKET_INFO PacketInfo;
	PNDIS_PACKET pPacket;

	pPacket = pTxBlk->pPacket;
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);

	if (RTMP_GET_PACKET_MGMT_PKT(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);

	if (IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if ((pTxBlk->TxFrameType == TX_LEGACY_FRAME) || (pTxBlk->TxFrameType == TX_AMSDU_FRAME) || (pTxBlk->TxFrameType == TX_MCAST_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
	}

	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	return TRUE;
}

INT32 mt_ate_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	INT32 ret = 0;
	UINT32 band_idx = HcGetBandByWdev(wdev);
	TMAC_INFO tmac_info;
	PQUEUE_ENTRY q_entry;
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;

	q_entry = RemoveHeadQueue(&tx_blk->TxPacketList);
	tx_blk->pPacket = QUEUE_ENTRY_TO_PACKET(q_entry);
	RTMP_SET_PACKET_WCID(tx_blk->pPacket, 0);

	/* Fill TX blk for ATE mode */
	ret = mt_ate_fill_offload_tx_blk(pAd, wdev, tx_blk);

	/* TMAC_INFO setup for ATE mode */
	ret = mt_ate_set_tmac_info(pAd, &tmac_info, band_idx);

	if (ret)
		return ret;

	return arch_ops->ate_hw_tx(pAd, &tmac_info, tx_blk);
}


#endif


#if !defined(COMPOS_TESTMODE_WIN)	/* 1todo too many OS private function */
static INT32 MT_ATEStartTx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	INT32 Ret = 0;
	UCHAR cntrol_band_idx = ATECtrl->control_band_idx;
	UCHAR *pate_pkt = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, pate_pkt);
	UCHAR cntl_ch = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, ControlChl);
	UCHAR ch = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, Channel);
	UINT32 mode = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, Mode);
	UINT32 tx_cnt = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, TxCount);
	UCHAR bw = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, BW);
	INT8 wdev_idx = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, wdev_idx);
	UINT32 Rate = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, Mcs);
	UINT32 duty_cycle = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, duty_cycle);
#ifdef ARBITRARY_CCK_OFDM_TX
	UINT32 tx_sel = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, TxAntennaSel);
	UINT8 phymode = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, PhyMode);
#endif
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: cntrol_band_idx:%u, ch:%x, cntl_ch:%x, wdev_idx:%x\n",
		__func__, cntrol_band_idx, ch, cntl_ch, wdev_idx));

	if (!pate_pkt)
		goto err0;

	/* TxRx swtich Recover */

	if (mode & ATE_TXFRAME) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s: already in TXFRAME mode now, tx is on-going!\n", __func__));
		goto err1;
	}

	MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, cntrol_band_idx);
#ifdef ARBITRARY_CCK_OFDM_TX
	if (IS_MT7615(pAd)) {
		MtATEInitCCK_OFDM_Path(pAd, cntrol_band_idx);

		if (phymode == MODE_CCK || phymode == MODE_OFDM)
			MtATESetCCK_OFDM_Path(pAd, tx_sel, cntrol_band_idx);
	}
#endif

	if (Rate == 32)
		mt_ate_mcs32_handle(pAd, ATECtrl->wcid_ref, bw);

	MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE, cntrol_band_idx);
	msleep(30);
	/* Stop send TX packets from upper layer */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/* The first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	/*  Disable PDMA */
	AsicSetWPDMA(pAd, PDMA_TX, 0);

	/* Polling TX/RX path until packets empty */
	if (if_ops->clean_trx_q)
		if_ops->clean_trx_q(pAd);

	if (mode & ATE_RXFRAME)
		MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, TRUE, cntrol_band_idx);

	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	/*  Enable PDMA */
	AsicSetWPDMA(pAd, PDMA_TX_RX, 1);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

	/* Calculate duty_cycle related parameter first */
	if (duty_cycle > 0)
		Ret = mt_ate_calculate_duty_cycle(pAd, cntrol_band_idx);

	/* Prepare Tx packet */
	if (if_ops->setup_frame)
		Ret = if_ops->setup_frame(pAd, QID_AC_BE);
	else
		Ret = MT_ATEGenPkt(pAd, pate_pkt, cntrol_band_idx);

	if (Ret)
		goto err0;

	/* Apply IPG setting to HW */
	Ret = mt_ate_apply_ipg_param(pAd, cntrol_band_idx);

	if (tx_cnt != 0xFFFFFFFF) {
#ifndef ATE_TXTHREAD
		tx_cnt += TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx,	TxDoneCount);
#endif
		TESTMODE_SET_PARAM(ATECtrl, cntrol_band_idx, TxCount, tx_cnt);
	}

	/* Tx Frame */
	mode |= ATE_TXFRAME;
	TESTMODE_SET_PARAM(ATECtrl, cntrol_band_idx, Mode, mode);

	if (if_ops->test_frame_tx)
		Ret = if_ops->test_frame_tx(pAd);
	else {
		struct _ATE_IPG_PARAM *ipg_param = TESTMODE_GET_PADDR(ATECtrl, cntrol_band_idx, ipg_param);
		struct _ATE_TX_TIME_PARAM *tx_time_param = TESTMODE_GET_PADDR(ATECtrl, cntrol_band_idx, tx_time_param);
		UINT32 ipg = ipg_param->ipg;
		UINT32 pkt_tx_time = tx_time_param->pkt_tx_time;

		if ((pkt_tx_time > 0) || (ipg > 0)) {
			PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *) pAd->PktTokenCb;
			UINT32 pkt_cnt, input_cnt;
			UINT32 ate_tx_cnt = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, TxCount);
			UINT32 txed_cnt = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, TxedCount);
			UINT32 pkt_ampdu_cnt = tx_time_param->pkt_ampdu_cnt;
			UINT32 pkt_enqueue_cnt = pktTokenCb->pkt_tx_tkid_max / 2;

			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: ate_tx_cnt=0x%x, pkt_ampdu_cnt=%d, pkt_enqueue_cnt=%d, pkt_tx_time=%d, ipg=%d\n",
				__func__, ate_tx_cnt, pkt_ampdu_cnt, pkt_enqueue_cnt, pkt_tx_time, ipg));
			MtATESetMacTxRx(pAd, ASIC_MAC_TX, FALSE, cntrol_band_idx);

			if (ate_tx_cnt > pkt_ampdu_cnt)
				input_cnt = ate_tx_cnt;
			else
				input_cnt = pkt_ampdu_cnt;

			input_cnt = input_cnt > pkt_enqueue_cnt ? pkt_enqueue_cnt : input_cnt;

			/* Enqueue packet in HW queue in advance */
			for (pkt_cnt = 0; pkt_cnt < input_cnt; pkt_cnt++) {
				MT_ATETxPkt(pAd, cntrol_band_idx);
				txed_cnt++;
			}

			TESTMODE_SET_PARAM(ATECtrl, cntrol_band_idx, TxedCount, txed_cnt);
			Ret = MT_ATETxControl(pAd, cntrol_band_idx, NULL);
			MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, cntrol_band_idx);
		} else
			Ret = MT_ATETxControl(pAd, cntrol_band_idx, NULL);
	}

	if (Ret)
		goto err0;

	ATECtrl->did_tx = 1;
err1:
	return Ret;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: Err %d, wdev_idx:%x\n", __func__, Ret, wdev_idx));
	return Ret;
}


static INT32 MT_ATEStartRx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	INT32 Ret = 0;
	UCHAR cntrol_band_idx = ATECtrl->control_band_idx;
	UINT32 mode = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, Mode);
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif

	if (mode & ATE_RXFRAME)
		goto err0;

	/* Firmware offloading CR need to msleep(30) Currently for the second NETDEV_STOP_QUEUE */
	MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE, cntrol_band_idx);
	msleep(30);
	/*   Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	AsicSetWPDMA(pAd, PDMA_TX, 0);

	if (if_ops->clean_trx_q)
		if_ops->clean_trx_q(pAd);

	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

	/* Turn on TX again if set before */
	if (mode & ATE_TXFRAME)
		MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, cntrol_band_idx);

	/* reset counter when iwpriv only */
	if (ATECtrl->bQAEnabled != TRUE) {
		ATECtrl->rx_stat.RxTotalCnt[0] = 0;

		if (IS_ATE_DBDC(pAd))
			ATECtrl->rx_stat.RxTotalCnt[1] = 0;
	}

	pAd->WlanCounters[0].FCSErrorCount.u.LowPart = 0;
	/* Enable PDMA */
	AsicSetWPDMA(pAd, PDMA_TX_RX, 1);
	MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, TRUE, cntrol_band_idx);
#ifdef CONFIG_HW_HAL_OFFLOAD
	MtCmdSetPhyCounter(pAd, 0, TESTMODE_BAND0);
	MtCmdSetPhyCounter(pAd, 1, TESTMODE_BAND0);

	if (IS_ATE_DBDC(pAd)) {
		MtCmdSetPhyCounter(pAd, 0, TESTMODE_BAND1);
		MtCmdSetPhyCounter(pAd, 1, TESTMODE_BAND1);
	}

#endif /* CONFIG_HW_HAL_OFFLOAD */
	msleep(30);
	mode |= ATE_RXFRAME;
	TESTMODE_SET_PARAM(ATECtrl, cntrol_band_idx, Mode, mode);

	if (if_ops->test_frame_rx)
		if_ops->test_frame_rx(pAd);

	ATECtrl->did_rx = 1;
err0:
	return Ret;
}


static INT32 MT_ATEStopTx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
#ifdef ATE_TXTHREAD
	INT32 thread_idx = 0;
#endif
	UCHAR cntrol_band_idx = ATECtrl->control_band_idx;
	UINT32 Mode = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, Mode);
	struct qm_ops *ops = pAd->qm_ops;
	struct _ATE_IPG_PARAM *ipg_param = TESTMODE_GET_PADDR(ATECtrl, cntrol_band_idx, ipg_param);
	struct _ATE_TX_TIME_PARAM *tx_time_param = TESTMODE_GET_PADDR(ATECtrl, cntrol_band_idx, tx_time_param);
	UINT32 ipg = ipg_param->ipg;
	UINT32 pkt_tx_time = tx_time_param->pkt_tx_time;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: cntrol_band_idx=%u\n", __func__, cntrol_band_idx));

	TESTMODE_SET_PARAM(ATECtrl, cntrol_band_idx, TxedCount, 0);
#ifdef ATE_TXTHREAD
	TESTMODEThreadStopTx(pAd, thread_idx);
#endif

	if ((Mode & ATE_TXFRAME) || (Mode == ATE_STOP)) {
		Mode &= ~ATE_TXFRAME;
		TESTMODE_SET_PARAM(ATECtrl, cntrol_band_idx, Mode, Mode);

		if ((pkt_tx_time > 0) || (ipg > 0)) {
			/* Flush SW queue */
			if (ops->sta_clean_queue)
				ops->sta_clean_queue(pAd, WCID_ALL);

			/* Clean per sta TX queue and disable STA pause CRs for transmitting packet */
			MtATESetCleanPerStaTxQueue(pAd, FALSE);
		}
#ifdef ARBITRARY_CCK_OFDM_TX
		if (IS_MT7615(pAd)) {
			MtATEInitCCK_OFDM_Path(pAd, cntrol_band_idx);
		}
#endif
	}

	return Ret;
}


static INT32 MT_ATEStopRx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	INT32 Ret = 0;
	UCHAR cntrol_band_idx = ATECtrl->control_band_idx;
	UINT32 Mode = TESTMODE_GET_PARAM(ATECtrl, cntrol_band_idx, Mode);

	Ret = MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE, cntrol_band_idx);
	Mode &= ~ATE_RXFRAME;
	TESTMODE_SET_PARAM(ATECtrl, cntrol_band_idx, Mode, Mode);

	if (if_ops->clean_trx_q)
		if_ops->clean_trx_q(pAd);

	return Ret;
}
#else
static INT32 MT_ATEStartTx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
#ifdef COMPOS_TESTMODE_WIN
	Ret = StartTx(pAd, 0, 0);
#endif
	ATECtrl->did_tx = 1;
	return Ret;
}

static INT32 MT_ATEStartRx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
#ifdef COMPOS_TESTMODE_WIN
	Ret = StartRx0(pAd);
	Ret = StartRx1(pAd);
#endif
	ATECtrl->did_rx = 1;
	return Ret;
}


static INT32 MT_ATEStopTx(RTMP_ADAPTER *pAd, UINT32 Mode)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
#ifdef COMPOS_TESTMODE_WIN
	Ret = StopTx(pAd);
#endif
	ATECtrl->Mode &= ~ATE_TXFRAME;
	return Ret;
}


static INT32 MT_ATEStopRx(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
#ifdef COMPOS_TESTMODE_WIN
	Ret = StopRx0(pAd);
	Ret = StopRx1(pAd);
#endif
	return Ret;
}
#endif


static INT32 MT_ATESetTxAntenna(RTMP_ADAPTER *pAd, UINT32 Ant)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: Ant = 0x%x, control_band_idx = %d\n",
		__func__, Ant, control_band_idx));

	/* For TXD setting, change to stream number when ATE set channel (MtCmdSetTxRxPath) */
	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, TxAntennaSel, Ant);

#if !defined(COMPOS_TESTMODE_WIN)/* 1       todo only 7603/7628 E1? */
#endif
	return Ret;
}


static INT32 MT_ATESetRxAntenna(RTMP_ADAPTER *pAd, UINT32 Ant)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: Ant = 0x%x, control_band_idx = %d\n",
		__func__, Ant, control_band_idx));

	/* After mt7615 its 4 bit mask for Rx0,1,2,3 */
	/* Send to FW and take effect when ATE set channel (MtCmdSetTxRxPath) */
	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, RxAntennaSel, Ant);

	return Ret;
}


static INT32 MT_ATESetTxFreqOffset(RTMP_ADAPTER *pAd, UINT32 FreqOffset)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	ATECtrl->RFFreqOffset = FreqOffset;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret = MtCmdSetFreqOffset(pAd, FreqOffset);
#else

	if ((IS_MT76x6(pAd)) &&
		!(ATECtrl->en_man_set_freq)) {
		UINT32 reg = 0;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("MT76x6 Manual Set Freq bk ori val\n"));
		MtTestModeBkCr(pAd, FREQ_OFFSET_MANUAL_ENABLE, TEST_HW_BKCR);
		MtTestModeBkCr(pAd, FREQ_OFFSET_MANUAL_VALUE, TEST_HW_BKCR);
		reg = (reg & 0xFFFF80FF) | (0x7F << 8);
		HW_IO_WRITE32(pAd, FREQ_OFFSET_MANUAL_ENABLE, reg);
		ATECtrl->en_man_set_freq = 1;
	}

	if (ATECtrl->en_man_set_freq) {
		UINT32 reg = 0;
		HW_IO_READ32(pAd, FREQ_OFFSET_MANUAL_VALUE, &reg);
		reg = (reg & 0xFFFF80FF) | (ATECtrl->RFFreqOffset << 8);
		HW_IO_WRITE32(pAd, FREQ_OFFSET_MANUAL_VALUE, reg);
	} else
		MtAsicSetRfFreqOffset(pAd, ATECtrl->RFFreqOffset);

#endif
	return Ret;
}


static INT32 MT_ATEGetTxFreqOffset(RTMP_ADAPTER *pAd, UINT32 *FreqOffset)
{
	INT32 Ret = 0;
#if	defined(COMPOS_TESTMODE_WIN)
	EXT_EVENT_ID_ATE_TEST_MODE_T *pResult = (EXT_EVENT_ID_ATE_TEST_MODE_T *)((UINT8 *) pAd->FWRspContent + sizeof(EVENT_RXD));
#endif
#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret = MtCmdGetFreqOffset(pAd, FreqOffset);
	os_msec_delay(30);
#if	defined(COMPOS_TESTMODE_WIN)
	*FreqOffset = OS_NTOHL(pResult->aucAteResult);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: FreqOffset:%X pResult->aucAteResult = %X\n",
			  __func__, *FreqOffset, pResult->aucAteResult));
#endif
#endif
	return Ret;
}

static INT32 MT_ATESetChannel(RTMP_ADAPTER *pAd,
					INT16 Value, UINT32 pri_sel,
					UINT32 reason,
					UINT32 Ch_Band)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR ctrl_ch = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UCHAR ch = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, Channel);
	UCHAR bw = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, BW);
	UCHAR wdev_idx = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, wdev_idx);
	UINT8 phymode = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, PhyMode);
	UINT32 tx_sel = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, TxAntennaSel);
	UINT32 rx_sel = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, RxAntennaSel);
	UINT32 out_band_freq = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, OutBandFreq);

	/* Backup several parameters before TSSI calibration */
	UCHAR ucPerPktBW_backup;
	UCHAR ucPhyMode_backup;
	UCHAR ucMcs_backup;
	UCHAR ucNss_backup;
	UINT32 u4TxLength_backup;

#ifdef TXBF_SUPPORT
	UINT32 iTxBf = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, iTxBf);
	UINT32 eTxBf = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, eTxBf);
#endif
	INT i = 0;
	UINT32 tx_stream_num = 0;
	UINT32 max_stream_num = 0;
	MT_SWITCH_CHANNEL_CFG ch_cfg;
	INT32 ch_offset = 0;
	const INT bw40_sel[] = { -2, 2};
#ifdef DOT11_VHT_AC
	const INT bw80_sel[] = { -6, -2, 2, 6};
	const INT bw160_sel[] = { -14, -10, -6, -2, 2, 6, 10, 14};
	UCHAR ch2 = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, Channel_2nd);
#endif
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#if !defined(COMPOS_TESTMODE_WIN)
	struct wifi_dev *pWdev = NULL;

	/* To update wdev setting according to ch_band */
	pWdev = pAd->wdev_list[wdev_idx];
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: wdev_idx=%d, ch=%d\n",
		__func__, wdev_idx, ch));

	if (!pWdev)
		goto err;

	pWdev->channel = ch;

	if (Ch_Band == 0)
		pWdev->PhyMode = PHYMODE_CAP_24G;
	else if (Ch_Band == 1)
		pWdev->PhyMode = PHYMODE_CAP_5G;
	else {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Error ch_band=%d!!\n", __func__, Ch_Band));
		return -1;
	}

	Ret = wdev_attr_update(pAd, pWdev);
#endif /* !defined(COMPOS_TESTMODE_WIN) */

	if (IS_ATE_DBDC(pAd))
		max_stream_num = cap->max_nss / 2 ? cap->max_nss / 2 : 1;
	else
		max_stream_num = cap->max_nss;

	/* 0: 20M; 1: 40M; 2: 80M; 3: 160M; 4: 10M; 5: 5M; 6: 80+80MHz */
	switch (bw) {
	case BW_40:
		if (pri_sel >= 2)
			goto err0;

		ctrl_ch = ch + bw40_sel[pri_sel];
		ch_offset = bw40_sel[pri_sel];

		if ((INT32)(ch + ch_offset) <= 0 || (INT32)(ch - ch_offset) <= 0)
			goto err1;

		break;
#ifdef DOT11_VHT_AC

	case BW_8080:
		if (pri_sel >= 8)
			goto err0;

		if ((ch2 < ch) && ch2) {
			UCHAR tmp = ch;
			ch = ch2;
			ch2 = tmp;
		}

		if (pri_sel < 4) {
			ctrl_ch = ch + bw80_sel[pri_sel];
			ch_offset = bw80_sel[pri_sel];

			if ((INT32)(ch + ch_offset) <= 0 || (INT32)(ch - ch_offset) <= 0)
				goto err1;
		} else {
			ctrl_ch = ch2 + bw80_sel[pri_sel - 4];
			ch_offset = bw80_sel[pri_sel - 4];

			if ((INT32)(ch2 + ch_offset) <= 0 || (INT32)(ch2 - ch_offset) <= 0)
				goto err1;
		}

		break;

	case BW_80:
		if (pri_sel >= 4)
			goto err0;

		ctrl_ch = ch + bw80_sel[pri_sel];
		ch_offset = bw80_sel[pri_sel];

		if ((INT32)(ch + ch_offset) <= 0 || (INT32)(ch - ch_offset) <= 0)
			goto err1;

		break;

	case BW_160:
		if (pri_sel >= 8)
			goto err0;

		ctrl_ch = ch + bw160_sel[pri_sel];
		ch_offset = bw160_sel[pri_sel];

		if ((INT32)(ch + ch_offset) <= 0 || (INT32)(ch - ch_offset) <= 0)
			goto err1;

		break;
#endif

	case BW_20:
	default:
		ctrl_ch = ch;
		break;
	}

	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, ControlChl, ctrl_ch);
	NdisZeroMemory(&ch_cfg, sizeof(ch_cfg));
	ch_cfg.ControlChannel = ctrl_ch;
	ch_cfg.CentralChannel = ch;
	ch_cfg.Bw = bw;
#ifdef DOT11_VHT_AC
	if (ch_cfg.Bw == BW_8080)
		ch_cfg.ControlChannel2 = ch2;

#endif

	switch (phymode) {
	case MODE_CCK:
	case MODE_OFDM:
#ifdef ARBITRARY_CCK_OFDM_TX
		if (IS_MT7615(pAd)) {
			tx_stream_num = max_stream_num;
		} else
#endif
		{
			/* To get TX max stream number from TX antenna bit mask
			    tx_sel=2 -> tx_stream_num=2
			    tx_sel=4 -> tx_stream_num=3
			    tx_sel=8 -> tx_stream_num=4
			*/
			for (i = max_stream_num; i > 0; i--) {
				if (tx_sel & BIT(i-1)) {
					tx_stream_num = i;
					break;
				}
			}
		}
		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
	case MODE_VHT:
#ifdef TXBF_SUPPORT
		if (iTxBf || eTxBf) {
			for (i = 0; i < 4; i++) {
				if (tx_sel & (1 << i))
					tx_stream_num++;
				else
					break;
			}
		} else
			tx_stream_num = max_stream_num;

#else
		tx_stream_num = max_stream_num;
#endif
		break;

	default:
		tx_stream_num = max_stream_num;
	}

	/* Set Rx ant 2/3 for band1 */
	if (control_band_idx)
		rx_sel = rx_sel << 2;

	tx_stream_num = tx_stream_num ? tx_stream_num : 1;
	tx_stream_num = tx_stream_num <= max_stream_num ? tx_stream_num : max_stream_num;

	/* What FW needs is Tx stream num and Rx antenna path */
	ch_cfg.TxStream = tx_stream_num;
	ch_cfg.RxStream = rx_sel;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: T/Rx_sel:%x/%x, Tx Stream:%x, phymode:%x\n",
		__func__, tx_sel, rx_sel, tx_stream_num, phymode));

#if defined(COMPOS_TESTMODE_WIN)
	switch (reason) {
	case CH_SWITCH_SCAN:
		ch_cfg.bScan = TRUE;
		ch_cfg.isMCC = FALSE;
		break;

	case CH_SWITCH_INTERNAL_USED_BY_FW_3:
		/* MCC */
		ch_cfg.bScan = FALSE;
		ch_cfg.isMCC = TRUE;
		break;

	case CH_SWITCH_BY_NORMAL_TX_RX:
	deafult:
		ch_cfg.bScan = FALSE;
		ch_cfg.isMCC = FALSE;
		break;
	} /* switch(reason) */

#else
	ch_cfg.bScan = FALSE;
#endif
	ch_cfg.BandIdx = control_band_idx;
	ch_cfg.Channel_Band = Ch_Band;
	ch_cfg.OutBandFreq = out_band_freq;

#ifdef MT7615
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	mt7615_apply_cal_data(pAd, ch_cfg);
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
#endif

	/* Align normal mode set channel flow */
	MtCmdChannelSwitch(pAd, ch_cfg);
	/* MtCmdSetTxRxPath will configure TX/RX CR setting by FW */
	MtCmdSetTxRxPath(pAd, ch_cfg);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: control_band_idx:%u, bw:%x, ch:%u, ctrl_ch:%u, cntl_ch2:%u, pri_sel:%x\n",
		__func__, control_band_idx, bw, ch, ctrl_ch, ch_cfg.ControlChannel2, pri_sel));

	ucPerPktBW_backup = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, PerPktBW);
	ucPhyMode_backup = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, PhyMode);
	ucMcs_backup = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, Mcs);
	ucNss_backup = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, Nss);
	u4TxLength_backup = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, TxLength);

	/* 160C/160NC TSSI workaround */
	Ret = MtATETssiTrainingProc(pAd, ch_cfg.Bw, control_band_idx);

	/* Recovery several parameter after TSSI calibration */
	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, PerPktBW, ucPerPktBW_backup);
	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, PhyMode, ucPhyMode_backup);
	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, Mcs, ucMcs_backup);
	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, Nss, ucNss_backup);
	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, TxLength, u4TxLength_backup);

#ifdef SINGLE_SKU_V2
#ifdef TXBF_SUPPORT
#ifdef MT_MAC
#if defined(MT7615) || defined(MT7622)
	TxPowerBfBackoffParaCtrl(pAd, ch_cfg.Channel_Band, ch_cfg.ControlChannel, ch_cfg.BandIdx);
#else
#endif /* defined(MT7615) || defined(MT7622) */
#endif /*MT_MAC*/
#endif /*TXBF_SUPPORT*/
#endif /*SINGLE_SKU_V2*/
	return Ret;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Invalid pri_sel:%x, Set Channel Fail\n", __func__, pri_sel));
err1:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Invalid Control Channel:%u|%u, Set Channel Fail\n", __func__, ctrl_ch, ch - ch_offset));
#if !defined(COMPOS_TESTMODE_WIN)
err:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Cannot get Wdev by idx:%d\n", __func__, wdev_idx));
#endif /* !defined(COMPOS_TESTMODE_WIN) */
	return -1;
}


static INT32 MT_ATESetBW(RTMP_ADAPTER *pAd, UINT16 system_bw, UINT16 per_pkt_bw)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	if (per_pkt_bw == BW_NUM) {
		if (system_bw == BW_8080)
			per_pkt_bw = BW_160;
		else
			per_pkt_bw = system_bw;
	} else {
		if (per_pkt_bw > system_bw)
			per_pkt_bw = system_bw;
	}

	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, BW, system_bw);
	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, PerPktBW, per_pkt_bw);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: System BW=%d, Per Packet BW=%d, control_band_idx=%d\n",
		__func__, system_bw, per_pkt_bw, control_band_idx));
	return Ret;
}


static INT32 mt_ate_set_duty_cycle(RTMP_ADAPTER *pAd, UINT32 value)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	UINT32 duty_cycle = value;
	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, duty_cycle, duty_cycle);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: Duty cycle=%d%%, control_band_idx=%d\n",
		__func__, duty_cycle, control_band_idx));
	return ret;
}


static INT32 mt_ate_set_pkt_tx_time(RTMP_ADAPTER *pAd, UINT32 value)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	struct _ATE_TX_TIME_PARAM *tx_time_param = TESTMODE_GET_PADDR(ATECtrl, control_band_idx, tx_time_param);

	tx_time_param->pkt_tx_time = value;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: Pkt Tx time=%dus, control_band_idx=%d\n",
		__func__, tx_time_param->pkt_tx_time, control_band_idx));
	return ret;
}


#if !defined(COMPOS_TESTMODE_WIN)/* 1       todo RX_BLK */
static INT32 MT_ATESampleRssi(RTMP_ADAPTER *pAd, RX_BLK *RxBlk)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_RX_STATISTIC *rx_stat =
			&ATECtrl->rx_stat;
	INT32 Ret = 0;
	INT i = 0;
	UINT32 ANT_NUM = pAd->Antenna.field.TxPath;

	for (i = 0; i < ANT_NUM; i++) {
		if (RxBlk->rx_signal.raw_rssi[i] != 0) {
			rx_stat->LastRssi[i] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&RxBlk->rx_signal.raw_rssi[i]), i);

			if (rx_stat->MaxRssi[i] <  rx_stat->LastRssi[i])
				rx_stat->MaxRssi[i] = rx_stat->LastRssi[i];

			if (rx_stat->MinRssi[i] >  rx_stat->LastRssi[i])
				rx_stat->MinRssi[i] =  rx_stat->LastRssi[i];

			rx_stat->AvgRssiX8[i] = (rx_stat->AvgRssiX8[i] - rx_stat->AvgRssi[i]) + rx_stat->LastRssi[i];
			rx_stat->AvgRssi[i] = rx_stat->AvgRssiX8[i] >> 3;
		}

		rx_stat->LastSNR[i] = RxBlk->rx_signal.raw_snr[i];
	}

	rx_stat->NumOfAvgRssiSample++;
	return Ret;
}
#endif


static UINT8 sigext_time_list[] = {
	0, /* CCK */
	6, /* OFDM */
	6, /* HTMIX */
	6, /* HTGREENFIELD */
	6, /* VHT */
};
static UINT8 mt_ate_get_sigext_time_by_phymode(UCHAR phy_mode)
{
	UINT8 sigext_time = 0;

	switch (phy_mode) {
	case MODE_CCK:
		sigext_time = sigext_time_list[MODE_CCK];
		break;

	case MODE_OFDM:
		sigext_time = sigext_time_list[MODE_OFDM];
		break;

	case MODE_HTMIX:
		sigext_time = sigext_time_list[MODE_HTMIX];
		break;

	case MODE_HTGREENFIELD:
		sigext_time = sigext_time_list[MODE_HTGREENFIELD];
		break;

	case MODE_VHT:
		sigext_time = sigext_time_list[MODE_VHT];
		break;

	default:
		sigext_time = sigext_time_list[MODE_OFDM];
		break;
	}

	return sigext_time;
}


static UINT16 slot_time_list[] = {
	9, /* CCK */
	9, /* OFDM */
	9, /* HTMIX */
	9, /* HTGREENFIELD */
	9, /* VHT */
};
static UINT16 mt_ate_get_slot_time_by_phymode(UCHAR phy_mode)
{
	UINT16 slot_time = 0;

	switch (phy_mode) {
	case MODE_CCK:
		slot_time = slot_time_list[MODE_CCK];
		break;

	case MODE_OFDM:
		slot_time = slot_time_list[MODE_OFDM];
		break;

	case MODE_HTMIX:
		slot_time = slot_time_list[MODE_HTMIX];
		break;

	case MODE_HTGREENFIELD:
		slot_time = slot_time_list[MODE_HTGREENFIELD];
		break;

	case MODE_VHT:
		slot_time = slot_time_list[MODE_VHT];
		break;

	default:
		slot_time = slot_time_list[MODE_OFDM];
		break;
	}

	return slot_time;
}


static UINT16 mt_ate_get_cw(UINT32 ipg, UINT16 slot_time)
{
	INT cnt = 0, val;
	val = (ipg + slot_time) / slot_time;

	while (val >>= 1)
		cnt++;

	if (cnt >= MAX_CW)
		cnt = MAX_CW;

	return cnt;
}


static INT32 mt_ate_get_ipg_param(RTMP_ADAPTER *pAd)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	struct _ATE_IPG_PARAM *ipg_param = TESTMODE_GET_PADDR(ATECtrl, control_band_idx, ipg_param);
	UCHAR phy_mode;
	UINT32 ipg, real_ipg;
	UINT8 sig_ext, aifsn;
	UINT16 slot_time, sifs_time, cw;

	phy_mode = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, PhyMode);
	ipg = ipg_param->ipg;
	sig_ext = mt_ate_get_sigext_time_by_phymode(phy_mode);
	slot_time = mt_ate_get_slot_time_by_phymode(phy_mode);
	sifs_time = DEFAULT_SIFS_TIME;
	/*
	 *  1. ipg = sig_ext + sifs_time + slot_time
	 *  2. ipg = sig_ext + sifs_time + aifsn * slot_time + ((1 << cw) - 1) * slot_time
	 *  If it's CCK mode, there's no need to consider sig_ext
	 *  And it's no need to count in backoff time in older ATE driver design, configure SIFS/SLOT only
	 *  Consider which ac queue will be used each case
	 */
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: Expected ipg=%d, control_band_idx=%d\n",
		__func__, ipg, control_band_idx));

	if (ipg < (sig_ext + sifs_time + slot_time)) {
		UINT32 duty_cycle = TESTMODE_GET_PARAM(ATECtrl,	control_band_idx, duty_cycle);

		ipg_param->ipg = 0;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Invalid IPG!! sig_ext=%d, slot_time=%d, sifs_time=%d\n"
				  "%s: Set ipg=%d\n",
				  __func__, sig_ext, slot_time, sifs_time, __func__, ipg));

		if (duty_cycle > 0) {
			duty_cycle = 0;
			TESTMODE_SET_PARAM(ATECtrl, control_band_idx, duty_cycle, duty_cycle);
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Invalid IPG with such duty_cycle and pkt_tx_time!!\n"
					  "%s: Set duty_cycle=%d\n", __func__, __func__, duty_cycle));
		}

		return ret;
	}

	ipg -= sig_ext;

	if (ipg <= (MAX_SIFS_TIME + slot_time)) {
		sifs_time = ipg - slot_time;
		aifsn = MIN_AIFSN;
		cw = 0;
	} else {
		cw = mt_ate_get_cw(ipg, slot_time);
		ipg -= ((1 << cw) - 1) * slot_time;
		aifsn = ipg / slot_time;

		if (aifsn >= MAX_AIFSN)
			aifsn = MAX_AIFSN;

		ipg -= aifsn * slot_time;

		if (ipg <= DEFAULT_SIFS_TIME)
			sifs_time = DEFAULT_SIFS_TIME;
		else if ((ipg > DEFAULT_SIFS_TIME) &&
				 (ipg <= MAX_SIFS_TIME))
			sifs_time = ipg;
		else
			sifs_time = MAX_SIFS_TIME;
	}

	real_ipg = sig_ext + sifs_time + aifsn * slot_time
			   + ((1 << cw) - 1) * slot_time;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: sig_ext=%d, slot_time=%d, sifs_time=%d, aifsn=%d, cw=%d\n",
			  __func__, sig_ext, slot_time, sifs_time, aifsn, cw));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: Real ipg=%d\n", __func__, real_ipg));
	ipg_param->sig_ext = sig_ext;
	ipg_param->slot_time = slot_time;
	ipg_param->sifs_time = sifs_time;
	ipg_param->aifsn = aifsn;
	ipg_param->cw = cw;
	ipg_param->txop = 0;
	return ret;
}


static INT32 mt_ate_set_ipg(RTMP_ADAPTER *pAd)
{
	INT32 ret = 0;

	ret = mt_ate_get_ipg_param(pAd);
	return ret;
}


static INT32 mt_ate_set_slot_time(RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: SlotTime:%d, SifsTime:%d, control_band_idx:%d\n",
		__func__,
		SlotTime,
		SifsTime,
		control_band_idx));
#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret = MtCmdATESetSlotTime(pAd, (UINT8)SlotTime,	(UINT8)SifsTime, RIFS_TIME, EIFS_TIME, control_band_idx);
#endif
	return Ret;
}


static INT32 MT_ATESetAIFS(RTMP_ADAPTER *pAd, CHAR Value)
{
	INT32 Ret = 0;
	UINT val = Value & 0x000000ff;
	/* Test mode use AC0 for TX */
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Value:%x\n", __func__, val));
	MtAsicSetWmmParam(pAd, 0, WMM_PARAM_AC_0, WMM_PARAM_AIFSN, val);
	return Ret;
}


static INT32 MT_ATESetPowerDropLevel(RTMP_ADAPTER *pAd, UINT32 PowerDropLevel)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: PowerDropLevel:%d, control_band_idx:%d\n",
		__func__, PowerDropLevel, control_band_idx));
#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret = MtCmdATESetPowerDropLevel(pAd, (UINT8)PowerDropLevel, control_band_idx);
#endif /* CONFIG_HW_HAL_OFFLOAD */
	return Ret;
}


static INT32 MT_ATESetTSSI(RTMP_ADAPTER *pAd, CHAR WFSel, CHAR Setting)
{
	INT32 Ret = 0;
	Ret = MtAsicSetTSSI(pAd, Setting, WFSel);
	return Ret;
}


static INT32 MT_ATELowPower(RTMP_ADAPTER *pAd, UINT32 Control)
{
	INT32 Ret = 0;
#if !defined(COMPOS_TESTMODE_WIN)

	if (Control)
		MlmeLpEnter(pAd);
	else
		MlmeLpExit(pAd);

#endif /* !defined(COMPOS_TESTMODE_WIN) */
	return Ret;
}


static INT32 MT_ATESetDPD(RTMP_ADAPTER *pAd, CHAR WFSel, CHAR Setting)
{
	/* !!TEST MODE ONLY!! Normal Mode control by FW and Never disable */
	/* WF0 = 0, WF1 = 1, WF ALL = 2 */
	INT32 Ret = 0;
	Ret = MtAsicSetDPD(pAd, Setting, WFSel);
	return Ret;
}


static INT32 MT_ATEStartTxTone(RTMP_ADAPTER *pAd, UINT32 Mode)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	return Ret;
}

static INT32 MT_ATEDBDCTxTone(RTMP_ADAPTER *pAd,
					UINT32 Control,
					UINT32 AntIndex,
					UINT32 ToneType,
					UINT32 ToneFreq,
					INT32 DcOffset_I,
					INT32 DcOffset_Q,
					UINT32 Band)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	MtCmdTxTone(pAd, control_band_idx, Control, AntIndex, ToneType, ToneFreq, DcOffset_I, DcOffset_Q, Band);
	return Ret;
}

static INT32 MT_ATETxCWTone(RTMP_ADAPTER *pAd, UINT32 Control)
{
	INT32 Ret = 0;
	UINT32 Channel = 0;
	UINT32 AntMask = 0;
	UINT32 AntIdx = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UINT8 band_idx = ATECtrl->control_band_idx;
	UINT32 Band = 0;
	INT32 pwr1 = 0, pwr2 = 0;
	INT32 DcOffset_I = 0, DcOffset_Q = 0;

	if (band_idx == -1)
		return -1;

	AntMask = TESTMODE_GET_PARAM(ATECtrl, band_idx, TxAntennaSel);
	Channel = TESTMODE_GET_PARAM(ATECtrl, band_idx, Channel);
	pwr1 = TESTMODE_GET_PARAM(ATECtrl, band_idx, RF_Power);
	pwr2 = TESTMODE_GET_PARAM(ATECtrl, band_idx, Digital_Power);
	DcOffset_I = TESTMODE_GET_PARAM(ATECtrl, band_idx, DcOffset_I);
	DcOffset_Q = TESTMODE_GET_PARAM(ATECtrl, band_idx, DcOffset_Q);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: Control = %d Channel=%d AntMask=0x%x\n",
			   __FUNCTION__, Control, Channel, AntMask));

   if (Channel <= 14)
	   Band = 2407000;
   else
	   Band = 5000000;

   /* Set RF and Digital Gain */
   for (AntIdx = 0; AntIdx <= 3; AntIdx++) {
		if (Control == 0) {
		    MtCmdTxTonePower(pAd, RF_AT_EXT_FUNCID_TX_TONE_RF_GAIN, 0,
						     AntIdx, band_idx);
		    MtCmdTxTonePower(pAd, RF_AT_EXT_FUNCID_TX_TONE_DIGITAL_GAIN, 0,
							 AntIdx, band_idx);
		   } else {
			if (AntMask & BIT(AntIdx)) {
				MtCmdTxTonePower(pAd, RF_AT_EXT_FUNCID_TX_TONE_RF_GAIN, pwr1,
							     AntIdx, band_idx);
				MtCmdTxTonePower(pAd, RF_AT_EXT_FUNCID_TX_TONE_DIGITAL_GAIN, pwr2,
								 AntIdx, band_idx);
			}
		   }
   }
   /* Send Command to firmware */
   MtCmdTxTone(pAd, band_idx, Control, AntMask, 0, 0,
			   (Control == 1) ? DcOffset_I : 0,
			   (Control == 1) ? DcOffset_Q : 0,
			   Band);

   /* Firmware currently ignores AntMask when starting Tx Tone,
	* and starts TX Tone on all the antennas. Take care of the
	* same in driver for now */
   if (Control == 1) {
	   for (AntIdx = 0; AntIdx <= 3; AntIdx++) {
			if (!(AntMask & BIT(AntIdx))) {
				UINT32 CRvalue = 0;
				PHY_IO_READ32(pAd, 0x10610, &CRvalue);
				PHY_IO_WRITE32(pAd, 0x10610,
							   CRvalue & ~BIT(16+AntIdx) & ~BIT(24+AntIdx));
			}
	   }
   }

   return Ret;
}

static INT32 MT_ATESetTxTonePower(RTMP_ADAPTER *pAd, INT32 pwr1, INT32 pwr2)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: pwr1:%d, pwr2:%d\n", __func__, pwr1, pwr2));
	return Ret;
}

static INT32 MT_ATESetDBDCTxTonePower(RTMP_ADAPTER *pAd, INT32 pwr1, INT32 pwr2, UINT32 AntIdx)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: pwr1:%d, pwr2:%d, AntIdx:%d, control_band_idx:%d\n",
		__func__, pwr1, pwr2, AntIdx, control_band_idx));
	MtCmdTxTonePower(pAd, RF_AT_EXT_FUNCID_TX_TONE_RF_GAIN, pwr1, AntIdx, (UINT8)control_band_idx);
	MtCmdTxTonePower(pAd, RF_AT_EXT_FUNCID_TX_TONE_DIGITAL_GAIN, pwr2, AntIdx, (UINT8)control_band_idx);
	return Ret;
}


static INT32 MT_ATEStopTxTone(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	return Ret;
}


static INT32 MT_ATEStartContinousTx(RTMP_ADAPTER *pAd, CHAR WFSel, UINT32 TxfdMode)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UINT32 Phymode = 0, BW = 0, Pri_Ch = 0, Rate = 0, Central_Ch = 0, ant_sel = 0;

	Phymode = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, PhyMode);
	BW = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, BW);
	Pri_Ch = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, ControlChl);
	Central_Ch = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, Channel);
	Rate = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, Mcs);
	ant_sel = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, TxAntennaSel);

	if (BW == ATE_BAND_WIDTH_8080)
		BW = 3;

	if (BW == ATE_BAND_WIDTH_160)
		BW = 4;

	{
		UINT32 Control = 1;
		MtCmdTxContinous(pAd, Phymode, BW, Pri_Ch, Central_Ch, Rate, ant_sel, TxfdMode, control_band_idx, Control);
	}
	return Ret;
}


static INT32 MT_ATEStopContinousTx(RTMP_ADAPTER *pAd, UINT32 TxfdMode)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UINT32 Phymode = 0, BW = 0, Pri_Ch = 0, Rate = 0, Central_Ch = 0, ant_sel;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	Phymode = TESTMODE_GET_PARAM(ATECtrl, control_band_idx,	PhyMode);
	BW = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, BW);
	Pri_Ch = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, ControlChl);
	Central_Ch = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, Channel);
	Rate = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, Mcs);
	ant_sel = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, TxAntennaSel);
	{
		UINT32 Control = 0;
		Ret = MtCmdTxContinous(pAd, Phymode, BW, Pri_Ch, Central_Ch, Rate, ant_sel, TxfdMode, control_band_idx,	Control);
	}
	return Ret;
}


static INT32 MT_OnOffRDD(struct _RTMP_ADAPTER *pAd, UINT32 rdd_idx, UINT32 rdd_in_sel, UINT32 is_start)
{
	INT32 Ret = 0;
	BOOLEAN arb_rx_on = FALSE;
	arb_rx_on = is_start ? TRUE : FALSE;
	MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, arb_rx_on, TESTMODE_BAND0);

	if (IS_ATE_DBDC(pAd))
		MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, arb_rx_on, TESTMODE_BAND1);

	Ret = MtCmdSetRDDTestExt(pAd, rdd_idx, rdd_in_sel, is_start);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s ARB Rx On:%x\n", __func__, arb_rx_on));
	return Ret;
}

#ifdef TXPWRMANUAL
static INT32 MT_ATESetTxPwrManual(RTMP_ADAPTER *pAd, BOOLEAN fgPwrManCtrl, UINT8 u1TxPwrModeManual, UINT8 u1TxPwrBwManual, UINT8 u1TxPwrRateManual, INT8 i1TxPwrValueManual, UCHAR Band)
{
	INT Ret = 0;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("-----------------------------------------------------------------------------\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Set wrong parameters\n", __func__));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" CCK   (0) only supports rate setting: 0 ~ 3 and no supports different BW\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" OFDM  (1) only supports rate setting: 0 ~ 7 and no supports different BW\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" HT20  (2) only supports rate setting: MCS 0 ~ 7 \n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" HT40  (2) only supports rate setting: MCS 0 ~ 8 \n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" VHT20 (3) only supports rate setting: 0 ~ 9\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" VHT40 (3) only supports rate setting: 0\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("-----------------------------------------------------------------------------\n"));


	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: fgPwrManCtrl:%d, u1TxPwrModeManual:%d, u1TxPwrBwManual:%d, u1TxPwrRateManual:%d, i1TxPwrValueManual:%d, Band:%d\n",
		__func__, fgPwrManCtrl, u1TxPwrModeManual, u1TxPwrBwManual,
		u1TxPwrRateManual, i1TxPwrValueManual, Band));


    /*BW : 0:BW20, 1: BW40, 2: BW80, 3: BW160*/
	if (u1TxPwrBwManual == 0)
		u1TxPwrBwManual = 20;
	else if (u1TxPwrBwManual == 1)
		u1TxPwrBwManual = 40;
	else if (u1TxPwrBwManual == 2)
		u1TxPwrBwManual = 80;
	else
		u1TxPwrBwManual = 160;

	/*QA tool: 0:CCK, 1:OFDM, 2:HT-MIXED, 3: HT-GREEN,4: VHT */
	/*FW:	   0:CCK,  1:OFDM, 2:HT, 3:VHT*/
	if (u1TxPwrModeManual == 2 || u1TxPwrModeManual == 3)
		u1TxPwrModeManual = 2;
	else if (u1TxPwrModeManual == 4)
		u1TxPwrModeManual = 3;


	/* sanity check for Other input parameter */
	switch (u1TxPwrModeManual) {
	case 0:
		if (u1TxPwrRateManual > 3 || u1TxPwrBwManual > 0)
			Ret = NDIS_STATUS_FAILURE;
		break;
	case 1:
		if (u1TxPwrRateManual > 7 || u1TxPwrBwManual > 0)
			Ret = NDIS_STATUS_FAILURE;
		break;
	case 2:
		if (u1TxPwrBwManual == 20) {
			if (u1TxPwrRateManual > 7)
				Ret = NDIS_STATUS_FAILURE;
		} else if (u1TxPwrBwManual == 40) {
			if (u1TxPwrRateManual > 8)
				Ret = NDIS_STATUS_FAILURE;
			} else
				Ret = NDIS_STATUS_FAILURE;
		break;
	case 3:
		if (u1TxPwrBwManual == 20) {
			if (u1TxPwrRateManual > 9)
				Ret = NDIS_STATUS_FAILURE;
		} else if (u1TxPwrBwManual == 40) {
			if (u1TxPwrRateManual > 0)
				Ret = NDIS_STATUS_FAILURE;
		} else
			Ret = NDIS_STATUS_FAILURE;
		break;
	default:
		Ret = TxPowerManualCtrl (pAd, fgPwrManCtrl, u1TxPwrModeManual, u1TxPwrBwManual, u1TxPwrRateManual, i1TxPwrValueManual, Band);
	}
	return Ret;
}
#endif

static INT32 MT_ATESetCfgOnOff(RTMP_ADAPTER *pAd, UINT32 Type, UINT32 Enable)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: Type:%d, Enable:%d, control_band_idx:%d\n",
		__func__, Type, Enable, control_band_idx));
#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret = MtCmdCfgOnOff(pAd, Type, Enable, control_band_idx);
#endif
	return Ret;
}

static INT32 MT_ATEGetCfgOnOff(RTMP_ADAPTER *pAd, UINT32 Type, UINT32 *Result)
{
	INT32 Ret = 0;
#if	defined(COMPOS_TESTMODE_WIN)
	UINT32 Value = 0;
	GET_TSSI_STATUS_T *pResult = (GET_TSSI_STATUS_T *)((UINT8 *)pAd->FWRspContent + sizeof(EVENT_RXD) + sizeof(EXT_EVENT_ATE_TEST_MODE_T));
#endif
#ifdef CONFIG_HW_HAL_OFFLOAD
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	Ret = MtCmdGetCfgOnOff(pAd, Type, control_band_idx, Result);
	os_msec_delay(30);
#endif
#if	defined(COMPOS_TESTMODE_WIN)
	*Result = OS_NTOHL(pResult->ucEnable);
#endif
	return Ret;
}

static INT32 MT_ATESetAntennaPort(RTMP_ADAPTER *pAd, UINT32 RfModeMask, UINT32 RfPortMask, UINT32 AntPortMask)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: RfModeMask:%d RfPortMask:%d AntPortMask:%d\n",
			  __func__, (UINT8)RfModeMask, (UINT8)RfPortMask,
			  (UINT8)AntPortMask));
#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret = MtCmdSetAntennaPort(pAd, (UINT8)RfModeMask, (UINT8)RfPortMask, (UINT8)AntPortMask);
#endif
	return Ret;
}


static INT32 MT_ATEFWPacketCMDClockSwitchDisable(
	RTMP_ADAPTER *pAd, UINT8 isDisable)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: isDsiable=%d\n", __func__, (UINT8)isDisable));
	Ret = MtCmdClockSwitchDisable(pAd, isDisable);
	return Ret;
}


static INT32 MT_ATESetRXFilterPktLen(RTMP_ADAPTER *pAd, UINT32 Enable, UINT32 RxPktLen)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: Enable:%d, control_band_idx:%d, RxPktLen:%d\n",
		__func__, Enable, control_band_idx, RxPktLen));
#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret =  MtCmdRxFilterPktLen(pAd, Enable, control_band_idx, RxPktLen);
#endif
	return Ret;
}


static INT32 MT_ATEGetTxPower(RTMP_ADAPTER *pAd, UINT32 Channel, UINT32 Ch_Band, UINT32 *EfuseAddr, UINT32 *Power)
{
	INT32 Ret = 0;
#if	defined(COMPOS_TESTMODE_WIN)
	EXT_EVENT_ID_GET_TX_POWER_T *pResult = (EXT_EVENT_ID_GET_TX_POWER_T *)((UINT8 *) pAd->FWRspContent + sizeof(EVENT_RXD));
#endif
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	EXT_EVENT_ID_GET_TX_POWER_T TxPowerResult;
	INT32 Type = 1;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: Channel:%d, control_band_idx:%d, Ch_Band:%d\n",
		__func__, Channel, control_band_idx, Ch_Band));
	Ret =  MtCmdGetTxPower(pAd, Type, Channel, control_band_idx, Ch_Band, &TxPowerResult);
	os_msec_delay(30);
#if defined(MT7615) || defined(MT7622)
	*EfuseAddr = TxPowerResult.ucEfuseAddr;
	*Power = TxPowerResult.ucEfuseContent;
#else
#endif /* defined(MT7615) || defined(MT7622) */
#if	defined(COMPOS_TESTMODE_WIN)
	*EfuseAddr = OS_NTOHL(pResult->ucEfuseAddr);
	*Power = OS_NTOHL(pResult->ucEfuseContent);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: EfuseAddr:%d Power:%d\n", __func__, *EfuseAddr, *Power));
	return Ret;
}


static INT32 MT_ATEBssInfoUpdate(RTMP_ADAPTER *pAd, UINT32 OwnMacIdx, UINT32 BssIdx, UCHAR *Bssid)
{
	INT32 Ret = 0;
#if !defined(COMPOS_TESTMODE_WIN)	/* TODO::UNIYSW */
	BSS_INFO_ARGUMENT_T bss_info_argument;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: OwnMacIdx:%d BssIdx:%d Bssid:%02x:%02x:%02x:%02x:%02x:%02x\n",
			  __func__, OwnMacIdx, BssIdx, PRINT_MAC(Bssid)));
	NdisZeroMemory(&bss_info_argument, sizeof(BSS_INFO_ARGUMENT_T));
	bss_info_argument.OwnMacIdx = OwnMacIdx;
	bss_info_argument.ucBssIndex = BssIdx;
	os_move_mem(bss_info_argument.Bssid, Bssid,	MAC_ADDR_LEN);
	bss_info_argument.ucBcMcWlanIdx = MCAST_WCID_TO_REMOVE;
	bss_info_argument.NetworkType = NETWORK_INFRA;
	bss_info_argument.u4ConnectionType = CONNECTION_INFRA_AP;
	bss_info_argument.CipherSuit = Ndis802_11WEPDisabled;
	bss_info_argument.bss_state = BSS_ACTIVE;
	bss_info_argument.u4BssInfoFeature = BSS_INFO_BASIC_FEATURE;
	Ret = AsicBssInfoUpdate(pAd, bss_info_argument);
#endif /* !defined(COMPOS_TESTMODE_WIN) */
	return Ret;
}


static INT32 MT_ATEDevInfoUpdate(RTMP_ADAPTER *pAd, UINT32 OwnMacIdx, UCHAR *Bssid)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: control_band_idx:%d OwnMacIdx:%d Bssid:%02x:%02x:%02x:%02x:%02x:%02x\n",
		__func__, control_band_idx, OwnMacIdx, PRINT_MAC(Bssid)));
#if !defined(COMPOS_TESTMODE_WIN)	/* TODO::UNIYSW */
	Ret = AsicDevInfoUpdate(pAd, OwnMacIdx, Bssid, control_band_idx, TRUE, DEVINFO_ACTIVE_FEATURE);
#endif /* !defined(COMPOS_TESTMODE_WIN) */
	return Ret;
}

static INT32 MT_SetFFTMode(struct _RTMP_ADAPTER *pAd, UINT32 mode)
{
	INT32 Ret = 0;
	return Ret;
}


#ifdef LOGDUMP_TO_FILE
static INT32 MT_ATERDDParseResult(struct _ATE_LOG_DUMP_ENTRY entry, INT idx, RTMP_OS_FD_EXT fd)
#else
static INT32 MT_ATERDDParseResult(struct _ATE_LOG_DUMP_ENTRY entry, INT idx)
#endif
{
	struct _ATE_RDD_LOG *result = &entry.log.rdd;
	UINT32 *pulse = (UINT32 *)result->aucBuffer;

	if (!result->byPass)
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("[RDD]0x%08x %08x\n", result->u4Prefix, result->u4Count));

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("[RDD]0x%08x %08x\n", pulse[0], pulse[1]));
	return 0;
}


INT32 MT_ATERFTestCB(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	INT32 ret = 0;
	EXT_EVENT_RF_TEST_RESULT_T *result = (EXT_EVENT_RF_TEST_RESULT_T *)Data;
	EXT_EVENT_RF_TEST_DATA_T *data = (EXT_EVENT_RF_TEST_DATA_T *)result->aucEvent;
	static INT total;
	static INT EventType;
	static UINT32 recal_type;
	BOOLEAN test_done = FALSE;

	/* Length of Event ACK */
	if (Length == sizeof(struct _EVENT_EXT_CMD_RESULT_T))
		test_done = TRUE;

	if (test_done)
		goto done;
#ifdef PRE_CAL_MT7622_SUPPORT
	if ((!pAd->bPreCalMode) && (!(pAd->ATECtrl.Mode & fATE_IN_RFTEST)))
#else
	if (!(pAd->ATECtrl.Mode & fATE_IN_RFTEST))
#endif /* PRE_CAL_MT7622_SUPPORT */
		return ret;
	result->u4FuncIndex = le2cpu32(result->u4FuncIndex);
	EventType = result->u4FuncIndex;

	switch (EventType) {
	case RDD_TEST_MODE:
		if (pAd->ATECtrl.en_log & fATE_LOG_RDD) {
			struct _ATE_RDD_LOG unit;
			struct _EVENT_WIFI_RDD_TEST_T *log =
				(struct _EVENT_WIFI_RDD_TEST_T *)Data;
			UINT64 *data = (UINT64 *)log->aucBuffer;
			INT i = 0;
			UINT len = 0;

			log->u4FuncLength = le2cpu32(log->u4FuncLength);
			log->u4Prefix = le2cpu32(log->u4Prefix);
			log->u4Count = le2cpu32(log->u4Count);

			len = (log->u4FuncLength - sizeof(struct _EVENT_WIFI_RDD_TEST_T)
				+ sizeof(log->u4FuncIndex) + sizeof(log->u4FuncIndex))>>3;

			if (pAd->ATECtrl.en_log & fATE_LOG_TEST) {
				const UINT dbg_len = (log->u4FuncLength - sizeof(struct _EVENT_WIFI_RDD_TEST_T) + sizeof(log->u4FuncIndex) + sizeof(log->u4FuncIndex)) >> 2;
				UINT32 *tmp = (UINT32 *)log->aucBuffer;

				for (i = 0; i < dbg_len; i++)
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							 ("RDD RAW DWORD%d:%08x\n", i, tmp[i]));

				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s: RDD FuncLen:%u, len:%u, prefix:%08x, cnt:%u\n",
						  __func__, log->u4FuncLength, len, log->u4Prefix, log->u4Count));
			}

			os_zero_mem(&unit, sizeof(unit));
			unit.u4Prefix = log->u4Prefix;
			unit.u4Count = log->u4Count;

			for (i = 0; i < len; i++) {
				NdisMoveMemory(unit.aucBuffer, data++, ATE_RDD_LOG_SIZE);
				MT_ATEInsertLog(pAd, (UCHAR *)&unit, fATE_LOG_RDD, sizeof(unit));
				/* byPass is used @ logDump, if the same event, don't dump same message */
				unit.byPass = TRUE;
			}
		}

		break;

#ifdef INTERNAL_CAPTURE_SUPPORT
	case GET_ICAP_CAPTURE_STATUS:
		if (IS_MT7615(pAd))
			ExtEventICapStatusHandler(pAd, Data, Length);
		break;

	case GET_ICAP_RAW_DATA:
		RTEnqueueInternalCmd(pAd, CMDTHRED_ICAP_DUMP_RAW_DATA, (VOID *)Data, Length);
		break;
#endif/* INTERNAL_CAPTURE_SUPPORT */

	case RE_CALIBRATION:
		if (data) {
			struct _ATE_LOG_RECAL re_cal;
			INT i = 0;
			UINT32 cal_idx = 0;
			UINT32 cal_type = 0;
			UINT32 len = 0;
			UINT32 *dump_tmp = (UINT32 *)data->aucData;
			struct _ATE_LOG_DUMP_CB *log_cb = NULL;
#ifdef PRE_CAL_MT7622_SUPPORT
			UINT32 *cal_log = NULL;

			if (pAd->bPreCalMode)
				os_alloc_mem(pAd, (UCHAR **)&cal_log, CAL_LOG_SIZE);
#endif /* PRE_CAL_MT7622_SUPPORT */
			data->u4CalIndex = le2cpu32(data->u4CalIndex);
			data->u4CalType = le2cpu32(data->u4CalType);
			result->u4PayloadLength = le2cpu32(result->u4PayloadLength);
			cal_idx = data->u4CalIndex;
			cal_type = data->u4CalType;
			len = result->u4PayloadLength;
			len = (len - sizeof(EXT_EVENT_RF_TEST_DATA_T)) >> 2;
			log_cb = &pAd->ATECtrl.log_dump[ATE_LOG_RE_CAL - 1];
			/* MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, */
			/* ("CalType:%x\n", cal_type)); */
			re_cal.cal_idx = cal_idx;
			re_cal.cal_type = cal_type;

			if (total == 0) {
				recal_type = cal_type;
				log_cb->recal_curr_type = recal_type;
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 ("[Recal][%08x][START]\n", recal_type));
			}

			total += result->u4PayloadLength;

			if ((cal_type == CAL_ALL) &&
				(total == CAL_ALL_LEN))
				test_done = TRUE;

			for (i = 0; i < len; i++) {
				dump_tmp[i] = le2cpu32(dump_tmp[i]);
				if (i & 0x1) {
					MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%08x\n", dump_tmp[i]));
					re_cal.cr_val = dump_tmp[i];
#ifdef PRE_CAL_MT7622_SUPPORT
					if (cal_log)
						cal_log[(i-1)/2] = dump_tmp[i];
#endif /* PRE_CAL_MT7622_SUPPORT */
					if (pAd->ATECtrl.en_log & fATE_LOG_RE_CAL)
						MT_ATEInsertLog(pAd, (UCHAR *)&re_cal, fATE_LOG_RE_CAL, sizeof(re_cal));
				} else {
					MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("[Recal][%08x][%08x]", cal_type, dump_tmp[i]));
					re_cal.cr_addr = dump_tmp[i];
				}
			}
#ifdef PRE_CAL_MT7622_SUPPORT
			if (pAd->bPreCalMode) {
				if (cal_type == TX_LPFG) {
					MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("[cal_type][TX_LPFG]len=%d\n", len));
					RTMPZeroMemory(pAd->CalTXLPFGImage, CAL_TXLPFG_SIZE);
					memcpy(pAd->CalTXLPFGImage, cal_log, CAL_TXLPFG_SIZE);
				} else if (cal_type == TX_DCIQC) {
					MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("[cal_type][TX_DCIQC]len=%d\n", len));
					RTMPZeroMemory(pAd->CalTXDCIQImage, CAL_TXDCIQ_SIZE);
					memcpy(pAd->CalTXDCIQImage, cal_log, CAL_TXDCIQ_SIZE);
				} else if (cal_type == TX_DPD_LINK) {
					UINT16 tmp_len = (len/2) * sizeof(UINT32);

					MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("[cal_type][TX_DPD], ofset=%x, len =%d\n",
						pAd->TxDpdCalOfst, tmp_len));
					memcpy(pAd->CalTXDPDImage + pAd->TxDpdCalOfst, cal_log, tmp_len);
					pAd->TxDpdCalOfst += tmp_len;
				}
				os_free_mem(cal_log);
			}
#endif /* PRE_CAL_MT7622_SUPPORT */
		}
		break;

	case CALIBRATION_BYPASS:
		break;

	default:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s: No RF Test Event %x Dump\n", __func__, result->u4FuncIndex));
		break;
	}

done:

	if (test_done) {
		if (EventType == RE_CALIBRATION) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("[Recal][%08x][END]\n", recal_type));
		}

		total = 0;
		EventType = 0;
		recal_type = 0;
	}

	return ret;
}


#ifdef ATE_TXTHREAD
static INT32 MT_ATEMPSRunStatCheck(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _HQA_MPS_CB *mps_cb = TESTMODE_GET_PADDR(ATECtrl, band_idx, mps_cb);
	struct _HQA_MPS_SETTING *mps_setting = NULL;
	UINT32 txed_cnt = TESTMODE_GET_PARAM(ATECtrl, band_idx, TxedCount);
	UINT32 tx_cnt = TESTMODE_GET_PARAM(ATECtrl, band_idx, TxCount);
	UINT32 idx = 0;
	INT32 ret = 0;

	if (!mps_cb)
		goto err0;

	mps_setting = mps_cb->mps_setting;

	if (!mps_setting)
		goto err0;

	idx = mps_cb->ref_idx;

	if ((mps_cb->stat & ATE_MPS_ITEM_RUNNING) && (txed_cnt >= tx_cnt)) {
		/* UCHAR mode = TESTMODE_GET_PARAM(ATECtrl, band_idx, Mode); */
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: MPS Item Finished idx:%d mps_cnt:%d\n", __func__, idx, mps_cb->mps_cnt));
		OS_SPIN_LOCK(&mps_cb->lock);
		mps_cb->stat = 0;
		OS_SPIN_UNLOCK(&mps_cb->lock);

		if (idx > mps_cb->mps_cnt) {
			UINT32 mode = TESTMODE_GET_PARAM(ATECtrl, band_idx, Mode);
			mode &= ~fATE_MPS;
			mps_cb->setting_inuse = FALSE;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s: MPS All Finished idx:%d mps_cnt:%d\n", __func__, idx, mps_cb->mps_cnt));
			TESTMODE_SET_PARAM(ATECtrl, band_idx, Mode, mode);
			ret = MT_MPSTxStop(pAd);
		}
	}

	return ret;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: mps_cb/mps_setting NULL %p %p\n", __func__, mps_cb, mps_setting));
	return -1;
}
#endif


static INT32 MT_ATEMPSLoadSetting(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ate_op = ATECtrl->ATEOp;
	struct _HQA_MPS_CB *mps_cb = TESTMODE_GET_PADDR(ATECtrl, band_idx, mps_cb);
	struct _HQA_MPS_SETTING *mps_setting = NULL;
	UCHAR *pate_pkt = TESTMODE_GET_PARAM(ATECtrl, band_idx, pate_pkt);
	INT idx = 0;
	UCHAR phymode = 0;
	CHAR ant_sel = 0;
	UCHAR mcs = 0;
	INT32 ret = 0;
	UINT32 pwr = 0;
	UINT32 pkt_len = 0;
	UINT32 pkt_cnt = 0;
	UINT32 nss = 0;
	UINT32 pkt_bw = 0;
	UINT32 Channel = TESTMODE_GET_PARAM(ATECtrl, band_idx, Channel);
	UINT32 Ch_Band = TESTMODE_GET_PARAM(ATECtrl, band_idx, Ch_Band);
	ATE_TXPOWER TxPower;
	os_zero_mem(&TxPower, sizeof(TxPower));

	if (!mps_cb)
		goto err0;

	mps_setting = mps_cb->mps_setting;

	if (!mps_setting)
		goto err0;

	OS_SPIN_LOCK(&mps_cb->lock);

	if (mps_cb->stat & ATE_MPS_ITEM_RUNNING)
		goto err1;

	mps_cb->stat |= ATE_MPS_ITEM_RUNNING;
	idx = mps_cb->ref_idx;

	if (idx > mps_cb->mps_cnt)
		goto err2;

	phymode = (mps_setting[idx].phy & 0x0f000000) >> 24;
	ant_sel = (mps_setting[idx].phy & 0x00ffff00) >> 8;
	mcs = (mps_setting[idx].phy & 0x000000ff);
	pwr = mps_setting[idx].pwr;
	pkt_len = mps_setting[idx].pkt_len;
	pkt_cnt = mps_setting[idx].pkt_cnt;
	nss = mps_setting[idx].nss;
	pkt_bw = mps_setting[idx].pkt_bw;
	TESTMODE_SET_PARAM(ATECtrl, band_idx, PhyMode, phymode);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, TxAntennaSel, ant_sel);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, Mcs, mcs);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, Nss, nss);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, PerPktBW, pkt_bw);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, TxLength,	pkt_len);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, TxCount, pkt_cnt);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, TxDoneCount, 0);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, TxedCount, 0);
	ATECtrl->TxPower0 = pwr;
	TxPower.Power = pwr;
	TxPower.Channel = Channel;
	TxPower.Dbdc_idx = band_idx;
	TxPower.Band_idx = Ch_Band;
	OS_SPIN_UNLOCK(&mps_cb->lock);
	ret = ate_op->SetTxPower0(pAd, TxPower);
	ATECtrl->need_set_pwr = TRUE;

	if (mps_cb->ref_idx != 1)
		ret = MT_ATEGenPkt(pAd, pate_pkt, band_idx);

	mps_cb->ref_idx++;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Item[%d], PhyMode:%x, TxPath:%x, Rate:%x, PktLen:%u, PktCount:%u, Pwr:%x\n",
			  idx, phymode, ant_sel, mcs, pkt_len, pkt_cnt, pwr));
	return ret;
err2:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: mps_cb->ref_idx %d mps_cnt %d\n", __func__, mps_cb->ref_idx, mps_cb->mps_cnt));
err1:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: item[%d] is running\n", __func__, mps_cb->ref_idx - 1));
	OS_SPIN_UNLOCK(&mps_cb->lock);
	return ret;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: mps_cb/mps_setting NULL %p %p\n", __func__, mps_cb, mps_setting));
	return -1;
}


static INT32 MT_ATEMPSInit(RTMP_ADAPTER *pAd)
{
	INT ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _HQA_MPS_CB *mps_cb = &ATECtrl->mps_cb;

	if (mps_cb->mps_setting)
		os_free_mem(mps_cb->mps_setting);

	NdisZeroMemory(mps_cb, sizeof(*mps_cb));
	mps_cb->setting_inuse = FALSE;
	mps_cb->mps_cnt = 0;
	mps_cb->band_idx = 0;
	mps_cb->stat = 0;
	mps_cb->ref_idx = 1;
	mps_cb->mps_setting = NULL;
	NdisAllocateSpinLock(pAd, &mps_cb->lock);
	return ret;
}


static INT32 MT_ATEMPSRelease(RTMP_ADAPTER *pAd)
{
	INT ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _HQA_MPS_CB *mps_cb = &ATECtrl->mps_cb;

	if (mps_cb->mps_setting)
		os_free_mem(mps_cb->mps_setting);

	mps_cb->mps_setting = NULL;
	mps_cb->setting_inuse = FALSE;
	mps_cb->mps_cnt = 0;
	mps_cb->stat = 0;
	mps_cb->band_idx = 0;
	mps_cb->ref_idx = 1;
	NdisFreeSpinLock(&mps_cb->lock);
	return ret;
}


INT32 MT_SetATEMPSDump(RTMP_ADAPTER *pAd, UINT32 band_idx)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _HQA_MPS_CB *mps_cb = &ATECtrl->mps_cb;
	struct _HQA_MPS_SETTING *mps_setting = mps_cb->mps_setting;
	UINT32 i = 0;
	mps_cb = TESTMODE_GET_PADDR(ATECtrl, band_idx, mps_cb);
	mps_setting = mps_cb->mps_setting;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s-band[%u]::\n", __func__, band_idx));

	if (!mps_setting)
		return -1;

	for (i = 1; i <= mps_cb->mps_cnt; i++) {
		UINT32 phy = (mps_setting[i].phy & ~0xf0ffffff) >> 24;
		UINT32 path = (mps_setting[i].phy & ~0xff0000ff) >> 8;
		UINT32 rate = (mps_setting[i].phy & ~0xffffff00);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Item[%d], PhyMode:%x, TxPath:%x, Rate:%x, PktLen:%u, PktCount:%u, Pwr:%x Nss:%u, Bw:%u\n",
				  i, phy, path, rate, mps_setting[i].pkt_len,
				  mps_setting[i].pkt_cnt, mps_setting[i].pwr,
				  mps_setting[i].nss, mps_setting[i].pkt_bw));
	}

	return 0;
}


static INT32 MT_MPSSetParm(RTMP_ADAPTER *pAd, enum _MPS_PARAM_TYPE type, INT32 items, UINT32 *data)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	INT32 i = 0;
	UINT32 mode = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, Mode);
	struct _HQA_MPS_CB *mps_cb = TESTMODE_GET_PADDR(ATECtrl, control_band_idx, mps_cb);
	struct _HQA_MPS_SETTING *mps_setting;

	if ((items > 1024) || (items == 0))
		goto MT_MPSSetParm_RET_FAIL;

	if (mode & fATE_MPS)
		goto MT_MPSSetParm_RET_FAIL;

	if (!mps_cb->mps_setting && !mps_cb->mps_cnt) {
		mps_cb->mps_cnt = items;
		mps_cb->band_idx = control_band_idx;
		ret = os_alloc_mem(pAd,	(UCHAR **)&mps_cb->mps_setting, sizeof(struct _HQA_MPS_SETTING) * (items + 1));

		if (ret == NDIS_STATUS_FAILURE)
			goto MT_MPSSetParm_RET_FAIL;

		NdisZeroMemory(mps_cb->mps_setting, sizeof(struct _HQA_MPS_SETTING) * (items + 1));
	}

	mps_setting = mps_cb->mps_setting;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: control_band_idx:%u, items:%d, Mode:%x, mps_cb:%p, mps_set:%p\n",
		__func__, control_band_idx, items, mode, mps_cb, mps_setting));

	switch (type) {
	case MPS_SEQDATA:
		mps_setting[0].phy = 1;

		for (i = 0; i < items; i++)
			mps_setting[i + 1].phy = data[i];

		break;

	case MPS_PHYMODE:
		mps_setting[0].phy = 1;

		for (i = 0; i < items; i++) {
			mps_setting[i + 1].phy &= 0xf0ffffff;
			mps_setting[i + 1].phy |= (data[i] << 24) & 0x0f000000;
		}

		break;

	case MPS_PATH:
		mps_setting[0].phy = 1;

		for (i = 0; i < items; i++) {
			mps_setting[i + 1].phy &= 0xff0000ff;
			mps_setting[i + 1].phy |= (data[i] << 8) & 0x00ffff00;
		}

		break;

	case MPS_RATE:
		mps_setting[0].phy = 1;

		for (i = 0; i < items; i++) {
			mps_setting[i + 1].phy &= 0xffffff00;
			mps_setting[i + 1].phy |= (0x000000ff & data[i]);
		}

		break;

	case MPS_PAYLOAD_LEN:
		mps_setting[0].pkt_len = 1;

		for (i = 0; i < items; i++) {
			if (data[i] > MAX_TEST_PKT_LEN)
				data[i] = MAX_TEST_PKT_LEN;
			else if (data[i] < MIN_TEST_PKT_LEN)
				data[i] = MIN_TEST_PKT_LEN;

			mps_setting[i + 1].pkt_len = data[i];
		}

		break;

	case MPS_TX_COUNT:
		mps_setting[0].pkt_cnt = 1;

		for (i = 0; i < items; i++)
			mps_setting[i + 1].pkt_cnt = data[i];

		break;

	case MPS_PWR_GAIN:
		mps_setting[0].pwr = 1;

		for (i = 0; i < items; i++)
			mps_setting[i + 1].pwr = data[i];

		break;

	case MPS_NSS:
		mps_setting[0].nss = 1;

		for (i = 0; i < items; i++)
			mps_setting[i + 1].nss = data[i];

		break;

	case MPS_PKT_BW:
		mps_setting[0].pkt_bw = 1;

		for (i = 0; i < items; i++)
			mps_setting[i + 1].pkt_bw = data[i];

		break;

	default:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s: unknown setting type\n", __func__));
		break;
	}

	return ret;
MT_MPSSetParm_RET_FAIL:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s-fail, control_band_idx:%u, items:%d, Mode:%x\n",
		__func__, control_band_idx, items, mode));
	return NDIS_STATUS_FAILURE;
}


static INT32 MT_MPSTxStart(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	struct _HQA_MPS_CB *mps_cb = TESTMODE_GET_PADDR(ATECtrl, control_band_idx, mps_cb);
	struct _HQA_MPS_SETTING *mps_setting = mps_cb->mps_setting;
	UINT32 mode = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, Mode);
	UINT32 mps_cnt = mps_cb->mps_cnt;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: control_band_idx:%u, items:%u\n",
		__func__, control_band_idx, mps_cnt));

	if (!mps_setting || !mps_cnt || (mode & ATE_MPS))
		goto MPS_START_ERR;

	if (mps_cb->setting_inuse)
		goto MPS_START_ERR;

	mode |= fATE_MPS;
	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, Mode, mode);
	mps_cb->ref_idx = 1;
	mps_cb->setting_inuse = TRUE;
	ret = MT_SetATEMPSDump(pAd, control_band_idx);
	ret = MT_ATEMPSLoadSetting(pAd, control_band_idx);
	ret = ATEOp->StartTx(pAd);
	return ret;
MPS_START_ERR:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s error, mode:0x%x, mps_cnt:%x, MPS_SETTING: %p\n",
			  __func__, mode, mps_cnt, mps_setting));
	return ret;
}


static INT32 MT_MPSTxStop(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UINT32 mode = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, Mode);
	struct _HQA_MPS_CB *mps_cb = TESTMODE_GET_PADDR(ATECtrl, control_band_idx, mps_cb);
	struct _HQA_MPS_SETTING *mps_setting = mps_cb->mps_setting;

	mode &= ~ATE_TXFRAME;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: control_band_idx:%u, Mode:%x, inuse:%x, setting_addr:%p\n",
		__func__, control_band_idx, mode, mps_cb->setting_inuse, mps_setting));

	if (!(mode & ATE_MPS) && mps_setting &&	!mps_cb->setting_inuse) {
		struct _HQA_MPS_SETTING **setting_addr = &(mps_cb->mps_setting);
		mps_cb->mps_cnt = 0;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: bf free mem %p\n", __func__, mps_setting));
		os_free_mem(*setting_addr);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: af free mem %p\n", __func__, mps_setting));
		*setting_addr = NULL;
	}

	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, Mode, mode);
	return ret;
}


static INT32 MT_ATESetAutoResp(RTMP_ADAPTER *pAd, UCHAR *mac, UCHAR mode)
{
	INT32 ret = 0;
	struct _ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UCHAR *sa = NULL;
#ifdef CONFIG_AP_SUPPORT
	sa = ATECtrl->Addr3;
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
#if !defined(COMPOS_TESTMODE_WIN)

	if (mode) {
		if (sa)
			NdisMoveMemory(sa, mac, MAC_ADDR_LEN);

		AsicDevInfoUpdate(
			pAd,
			0x0,
			mac,
			control_band_idx,
			TRUE,
			DEVINFO_ACTIVE_FEATURE);
	} else {
		AsicDevInfoUpdate(
			pAd,
			0x0,
			pAd->CurrentAddress,
			control_band_idx,
			TRUE,
			DEVINFO_ACTIVE_FEATURE);
	}

#endif /* !defined(COMPOS_TESTMODE_WIN) */
	return ret;
}


static INT32 MT_EfuseGetFreeBlock(RTMP_ADAPTER *pAd, UINT32 GetFreeBlock, UINT32 *Value)
{
	INT32 Ret = 0;
#if	defined(COMPOS_TESTMODE_WIN)
	EXT_EVENT_EFUSE_FREE_BLOCK_T *pResult =	(EXT_EVENT_EFUSE_FREE_BLOCK_T *)((UINT8 *) pAd->FWRspContent + sizeof(EVENT_RXD));
#endif
	Ret = MtCmdEfuseFreeBlockCount(pAd, GetFreeBlock, Value);
#if	defined(COMPOS_TESTMODE_WIN)
	/* workaround for MtCmdEfuseFreeBlockCount not waiting event back when repoen QA second times */
	RTMPusecDelay(30000);
	*Value = OS_NTOHL(pResult->ucFreeBlockNum);
#endif
	return Ret;
}




static INT32 MT_RfRegWrite(RTMP_ADAPTER *pAd, UINT32 WFSel, UINT32 Offset, UINT32 Value)
{
	INT32 Ret = 0;
	Ret = MtCmdRFRegAccessWrite(pAd, WFSel, Offset, Value);
	return Ret;
}


static INT32 MT_RfRegRead(RTMP_ADAPTER *pAd, UINT32 WFSel, UINT32 Offset, UINT32 *Value)
{
	INT32 Ret = 0;
#if	defined(COMPOS_TESTMODE_WIN)
	EXT_CMD_RF_REG_ACCESS_T *pResult = (EXT_CMD_RF_REG_ACCESS_T *)((UINT8 *)pAd->FWRspContent + sizeof(EVENT_RXD));
#endif
	Ret = MtCmdRFRegAccessRead(pAd, WFSel, Offset, Value);
#if	defined(COMPOS_TESTMODE_WIN)
	*Value = OS_NTOHL(pResult->u4Data);
#endif
	return Ret;
}


static INT32 MT_GetFWInfo(RTMP_ADAPTER *pAd, UCHAR *FWInfo)
{
	struct fwdl_ctrl *ctrl = &pAd->MCUCtrl.fwdl_ctrl;
	UCHAR op_mode;
	UINT32 loop;
	UCHAR date[8] = {'\0'};
	UCHAR time[6] = {'\0'};
	UCHAR *kernel_info = NULL;
	UINT8 month = 0;
	UCHAR *month_array[12] = {
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec",
	};

	/* Get information from kernel */
	for (loop = 0; loop < 12; loop++) {
		kernel_info = strstr(utsname()->version, month_array[loop]);

		if (kernel_info)
			break;
	}

	op_mode = (UCHAR)
			  pAd->OpMode;	/* 0: STA, 1: AP, 2: ADHOC, 3: APSTA */
	/* Driver build time */
	os_move_mem(&time[0], kernel_info + 7, 2);
	os_move_mem(&time[2], kernel_info + 10, 2);
	os_move_mem(&time[4], kernel_info + 13, 2);
	/* Driver build date */
	os_move_mem(&date[0], kernel_info + 20, 4);
	os_move_mem(&date[6], kernel_info + 4, 2);

	for (loop = 0; loop < 12; loop++) {
		if (os_cmp_mem(month_array[loop], kernel_info, 3) == 0) {
			month = loop + 1;
			break;
		}
	}

	date[4] = month / 10 % 10 + '0';
	date[5] = month % 10 + '0';
	/* Command combination */
	os_move_mem(FWInfo, &op_mode, sizeof(op_mode));
	os_move_mem((FWInfo + sizeof(op_mode)), &date, sizeof(date));
	os_move_mem((FWInfo + sizeof(op_mode) + sizeof(date)), &time, sizeof(time));
	os_move_mem((FWInfo + sizeof(op_mode) + sizeof(date) + sizeof(time)), \
		ctrl->fw_profile[WM_CPU].source.img_ptr + ctrl->fw_profile[WM_CPU].source.img_len - 19, 15);
	return 0;
}


#ifdef TXBF_SUPPORT
/* ==========================================================================
 *  Description:
 *	Set ATE Tx Beamforming mode
 *
 *	Return:
 *		TRUE if all parameters are OK, FALSE otherwise
 * ==========================================================================
 */


#ifdef MT_MAC
/*
 *==========================================================================
 *   Description:
 *	Enable sounding trigger
 *
 *	Return:
 *		TRUE if all parameters are OK, FALSE otherwise
 *==========================================================================
*/
INT MT_SetATESoundingProc(RTMP_ADAPTER *pAd, UCHAR SDEnFlg)
{
	/* struct _ATE_CTRL *AteCtrl = &pAd->ATECtrl; */
	/* Enable sounding trigger in FW */
	/* return CmdETxBfSoundingPeriodicTriggerCtrl(pAd, BSSID_WCID, SDEnFlg); */
	/* return MtCmdETxBfSoundingPeriodicTriggerCtrl(pAd, BSSID_WCID, SDEnFlg, AteCtrl->BW); */
	return -1;
}


static INT32 MT_ATEStartTxSKB(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	INT32 Ret = 0;
	UCHAR control_band_idx = ATECtrl->control_band_idx;
	UCHAR *pate_pkt = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, pate_pkt);
	UCHAR cntl_ch = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, ControlChl);
	UCHAR ch = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, Channel);
	UINT32 mode = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, Mode);
	UINT32 tx_cnt = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, TxCount);
	UCHAR bw = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, BW);
	/* MT_SWITCH_CHANNEL_CFG ch_cfg; */
	INT8 wdev_idx = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, wdev_idx);
#ifdef ARBITRARY_CCK_OFDM_TX
	UINT32 tx_sel = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, TxAntennaSel);
	UINT8 phymode = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, PhyMode);
#endif
	UINT32 Rate = TESTMODE_GET_PARAM(ATECtrl, control_band_idx, Mcs);
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
	union WTBL_DW5 wtbl_txcap;
#else
	union WTBL_2_DW9 wtbl_txcap;
#endif
	UINT32 DwMask = 0;
	UINT32 Ring = 0;
	CMD_WTBL_RAW_DATA_RW_T rWtblRawDataRw = {0};
	rWtblRawDataRw.u2Tag = WTBL_RAW_DATA_RW;
	rWtblRawDataRw.u2Length = sizeof(CMD_WTBL_RAW_DATA_RW_T);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: control_band_idx:%u, ch:%x, cntl_ch:%x, wdev_idx:%x\n",
		__func__, control_band_idx, ch, cntl_ch, wdev_idx));

	if (!pate_pkt)
		goto err0;

	/* TxRx swtich Recover */

	if (mode & ATE_TXFRAME)
		goto err1;

	MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, control_band_idx);
#ifdef ARBITRARY_CCK_OFDM_TX
	if (IS_MT7615(pAd)) {
		MtATEInitCCK_OFDM_Path(pAd, control_band_idx);

		if (phymode == MODE_CCK || phymode == MODE_OFDM)
			MtATESetCCK_OFDM_Path(pAd, tx_sel, control_band_idx);
	}
#endif

	if (Rate == 32) {
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
		DwMask = ~(3 << 12); /* only update fcap bit[13:12] */
		wtbl_txcap.field.fcap = bw;
		/* WTBLDW5 */
		WtblDwSet(pAd, ATECtrl->wcid_ref, 1, 5, DwMask, wtbl_txcap.word);
#else
		DwMask = ~(3 << 14); /* only update fcap bit[15:14] */
		wtbl_txcap.field.fcap = bw;
		/* WTBL2DW9 */
		WtblDwSet(pAd, ATECtrl->wcid_ref, 2, 9, DwMask, wtbl_txcap.word);
#endif
	}

	MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE, control_band_idx);
	msleep(30);
	/*   Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID;
			 IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(
					pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_SET_FLAG(pAd,
				  fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

	if (mode & ATE_RXFRAME)
		MtATESetMacTxRx(pAd, ASIC_MAC_RX_RXV, TRUE, control_band_idx);

	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID;
			 IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(
					pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

	/* Prepare Tx packet */
	if (if_ops->setup_frame)
		Ret = if_ops->setup_frame(pAd, QID_AC_BE);
	else
		Ret = MT_ATEGenPkt(pAd, pate_pkt, control_band_idx);

	if (Ret)
		goto err0;

	if (tx_cnt != 0xFFFFFFFF) {
#ifndef ATE_TXTHREAD
		tx_cnt += TESTMODE_GET_PARAM(ATECtrl, control_band_idx,	TxDoneCount);
#endif
		TESTMODE_SET_PARAM(ATECtrl, control_band_idx, TxCount, tx_cnt);
	}

	/* Tx Frame */
	mode |= ATE_TXFRAME;
	TESTMODE_SET_PARAM(ATECtrl, control_band_idx, Mode, mode);

	if (if_ops->test_frame_tx)
		Ret = if_ops->test_frame_tx(pAd);
	else {
		MtATESetMacTxRx(pAd, ASIC_MAC_TX, FALSE, control_band_idx);

		for (Ring = 0; Ring < 2000; Ring++)
			MT_ATETxPkt(pAd, control_band_idx);

		Ret = MT_ATETxControl(pAd, control_band_idx, NULL);
		MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, control_band_idx);
	}

	if (Ret)
		goto err0;

	ATECtrl->did_tx = 1;
err1:
	return Ret;
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Err %d, wdev_idx:%x\n", __func__, Ret, wdev_idx));
	return Ret;
}
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */




#ifdef RTMP_MAC_PCI
static INT32 pci_ate_init(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT ret = 0;
	MTWF_LOG(DBG_CAT_TEST,  DBG_SUBCAT_ALL,	DBG_LVL_TRACE, ("%s\n", __func__));

	if (!ATECtrl->pate_pkt)
		ret = os_alloc_mem(pAd,	(PUCHAR *)&ATECtrl->pate_pkt, ATE_TESTPKT_LEN);

	if (ret)
		goto err0;

	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
	return NDIS_STATUS_SUCCESS;
err0:
	MTWF_LOG(DBG_CAT_TEST,  DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s: Allocate test packet fail at pakcet\n", __func__));
	return NDIS_STATUS_FAILURE;
}


static INT32 pci_clean_q(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_TEST,  DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	return NDIS_STATUS_SUCCESS;
}

static INT32 pci_ate_leave(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	MTWF_LOG(DBG_CAT_TEST,  DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	NICReadEEPROMParameters(pAd, NULL);
	NICInitAsicFromEEPROM(pAd);

	if (ATECtrl->pate_pkt) {
		os_free_mem(ATECtrl->pate_pkt);
		ATECtrl->pate_pkt = NULL;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif




#ifdef RTMP_MAC_PCI
static UINT32 MT_TestModeIfOpInit_PCI(struct _ATE_IF_OPERATION *ATEIfOps)
{
	UINT32 Status = 0;
	ATEIfOps->init = pci_ate_init;
	ATEIfOps->clean_trx_q = pci_clean_q;
	ATEIfOps->setup_frame = NULL;
	ATEIfOps->test_frame_tx = NULL;
	ATEIfOps->test_frame_rx = NULL;
	ATEIfOps->ate_leave = pci_ate_leave;
	return Status;
}

#elif defined(RTMP_MAC_USB)
static UINT32 MT_TestModeIfOpInit_USB(struct _ATE_IF_OPERATION *ATEIfOps)
{
	UINT32 Status = 0;
	ATEIfOps->init = usb_ate_init;
	ATEIfOps->clean_trx_q = usb_clean_q;
	ATEIfOps->setup_frame = usb_setup_frame;
	ATEIfOps->test_frame_tx = usb_test_frame_tx;
	ATEIfOps->test_frame_rx = usb_test_frame_rx;
	ATEIfOps->ate_leave = usb_ate_leave;
	return Status;
}
#elif defined(RTMP_MAC_SDIO)
static UINT32 MT_TestModeIfOpInit_SDIO(
	struct _ATE_IF_OPERATION *ATEIfOps)
{
	UINT32 Status = 0;
	ATEIfOps->init = sdio_ate_init;
	ATEIfOps->clean_trx_q = sdio_clean_q;
	ATEIfOps->setup_frame = sdio_setup_frame;
	ATEIfOps->test_frame_tx = sdio_test_frame_tx;
	ATEIfOps->test_frame_rx = NULL;
	ATEIfOps->ate_leave = sdio_test_leave;
	return Status;
}
#else
static UINT32 MT_TestModeIfOpInit_NotSupport(struct _ATE_IF_OPERATION *ATEIfOps)
{
	UINT32 Status = 0;
	ATEIfOps->init = NULL;
	ATEIfOps->clean_trx_q = NULL;
	ATEIfOps->setup_frame = NULL;
	ATEIfOps->test_frame_tx = NULL;
	ATEIfOps->test_frame_rx = NULL;
	ATEIfOps->ate_leave = NULL;
	return Status;
}
#endif


static UINT32 MT_TestModeIfOpInit(RTMP_ADAPTER *pAd)
{
	UINT32 Status = 0;
	struct _ATE_CTRL *AteCtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *ATEIfOps = NULL;
	os_alloc_mem(pAd, (PUCHAR *)&ATEIfOps, sizeof(*ATEIfOps));
#if defined(RTMP_MAC_PCI)
	MT_TestModeIfOpInit_PCI(ATEIfOps);
#elif defined(RTMP_MAC_SDIO)
	MT_TestModeIfOpInit_SDIO(ATEIfOps);
#elif defined(RTMP_MAC_USB)
	MT_TestModeIfOpInit_USB(ATEIfOps);
#else
	MT_TestModeIfOpInit_NotSupport(ATEIfOps);
#endif
	AteCtrl->ATEIfOps = ATEIfOps;
	return Status;
}


static UINT32 MT_TestModeOpInit(RTMP_ADAPTER *pAd)
{
	UINT32 Status = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *AteOp = NULL;
	os_alloc_mem(pAd, (PUCHAR *)&AteOp, sizeof(*AteOp));
	AteOp->ATEStart = MT_ATEStart;
	AteOp->ATEStop = MT_ATEStop;
	AteOp->StartTx = MT_ATEStartTx;
	AteOp->StartRx = MT_ATEStartRx;
	AteOp->StopTx = MT_ATEStopTx;
	AteOp->StopRx = MT_ATEStopRx;
	AteOp->SetTxPower0 = MT_ATESetTxPower0;
	AteOp->SetTxPower1 = MT_ATESetTxPower1;
	AteOp->SetTxPower2 = MT_ATESetTxPower2;
	AteOp->SetTxPower3 = MT_ATESetTxPower3;
	AteOp->SetTxForceTxPower = MT_ATESetForceTxPower;
	AteOp->SetTxPowerX = MT_ATESetTxPowerX;
	AteOp->SetTxAntenna = MT_ATESetTxAntenna;
	AteOp->SetRxAntenna = MT_ATESetRxAntenna;
	AteOp->SetTxFreqOffset = MT_ATESetTxFreqOffset;
	AteOp->GetTxFreqOffset = MT_ATEGetTxFreqOffset;
	AteOp->SetChannel = MT_ATESetChannel;
	AteOp->SetBW = MT_ATESetBW;
	AteOp->SetDutyCycle = mt_ate_set_duty_cycle;
	AteOp->SetPktTxTime = mt_ate_set_pkt_tx_time;
#ifdef PRE_CAL_MT7622_SUPPORT
	AteOp->TxDPDTest7622 = MtATE_DPD_Cal_Store_Proc_7622;
#endif /*PRE_CAL_MT7622_SUPPORT*/
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	AteOp->RxSelfTest = MtATE_DCOC_Cal_Store_Proc;
	AteOp->TxDPDTest = MtATE_DPD_Cal_Store_Proc;
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
#ifdef PRE_CAL_TRX_SET2_SUPPORT
	AteOp->PreCalTest = MtATE_Pre_Cal_Proc;
#endif /* PRE_CAL_TRX_SET2_SUPPORT */
#if !defined(COMPOS_TESTMODE_WIN)/* 1       todo RX_BLK */
	AteOp->SampleRssi = MT_ATESampleRssi;
#endif
	AteOp->SetIPG = mt_ate_set_ipg;
	AteOp->SetSlotTime = mt_ate_set_slot_time;
	AteOp->SetAIFS = MT_ATESetAIFS;
	AteOp->SetPowerDropLevel = MT_ATESetPowerDropLevel;
	AteOp->SetTSSI = MT_ATESetTSSI;
	AteOp->LowPower = MT_ATELowPower;
	AteOp->SetDPD = MT_ATESetDPD;
	AteOp->StartTxTone = MT_ATEStartTxTone;
	AteOp->DBDCTxTone = MT_ATEDBDCTxTone;
#ifdef TXPWRMANUAL
	AteOp->SetTxPwrManual = MT_ATESetTxPwrManual;
#endif
	AteOp->SetDBDCTxTonePower = MT_ATESetDBDCTxTonePower;
	AteOp->SetTxTonePower = MT_ATESetTxTonePower;
	AteOp->TxCWTone = MT_ATETxCWTone;
	AteOp->StopTxTone = MT_ATEStopTxTone;
	AteOp->StartContinousTx = MT_ATEStartContinousTx;
	AteOp->StopContinousTx = MT_ATEStopContinousTx;
	AteOp->RfRegWrite = MT_RfRegWrite;
	AteOp->RfRegRead = MT_RfRegRead;
	AteOp->EfuseGetFreeBlock = MT_EfuseGetFreeBlock;
	AteOp->GetFWInfo = MT_GetFWInfo;
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	AteOp->SetATETxSoundingProc = MT_SetATESoundingProc;
	AteOp->StartTxSKB = MT_ATEStartTxSKB;
#endif /* MT_MAC */
	AteOp->SetICapStart = MT_ATESetICapStart;
	AteOp->GetICapStatus = MT_ATEGetICapStatus;
	AteOp->GetICapIQData = MT_ATEGetICapIQData;
	AteOp->MPSSetParm = MT_MPSSetParm,
	AteOp->MPSTxStart = MT_MPSTxStart,
	AteOp->MPSTxStop = MT_MPSTxStop,
	AteOp->SetAutoResp = MT_ATESetAutoResp,
	AteOp->SetFFTMode = MT_SetFFTMode,
	AteOp->onOffRDD = MT_OnOffRDD,
	AteOp->SetCfgOnOff = MT_ATESetCfgOnOff,
	AteOp->GetCfgOnOff = MT_ATEGetCfgOnOff,
	AteOp->SetRXFilterPktLen = MT_ATESetRXFilterPktLen,
	AteOp->GetTxPower = MT_ATEGetTxPower,
	AteOp->BssInfoUpdate = MT_ATEBssInfoUpdate,
	AteOp->DevInfoUpdate = MT_ATEDevInfoUpdate,
	AteOp->LogOnOff = MT_ATELogOnOff,
	AteOp->SetAntennaPort = MT_ATESetAntennaPort,
	AteOp->ClockSwitchDisable = MT_ATEFWPacketCMDClockSwitchDisable,
	ATECtrl->ATEOp = AteOp;
	return Status;
}


INT32 MtTestModeInit(RTMP_ADAPTER *pAd)
{
	INT32 Status = 0;
	Status = MT_TestModeOpInit(pAd);
	Status = MT_TestModeIfOpInit(pAd);
	pAd->ATECtrl.pate_pkt = NULL;
	RTMP_OS_TASK_INIT(&pAd->LbCtrl.LoopBackTxTask, "ATE_LoopBackTask", pAd);
	/*Unify*/
#ifdef RTMP_PCI_SUPPORT

	if (IS_PCI_INF(pAd))
		RTMP_OS_INIT_COMPLETION(&pAd->LbCtrl.LoopBackPCITxEvent);

#endif
	RTMP_OS_INIT_COMPLETION(&pAd->LbCtrl.LoopBackEvent);
	os_zero_mem(&pAd->LbCtrl.LoopBackResult, sizeof(struct _LOOPBACK_RESULT));
	pAd->LbCtrl.DebugMode = FALSE;
	pAd->LbCtrl.LoopBackRunning = FALSE;
	pAd->LbCtrl.LoopBackWaitRx = FALSE;
	pAd->LbCtrl.LoopBackUDMA = FALSE;
	NdisAllocateSpinLock(pAd, &pAd->LbCtrl.LoopBackLock);
#ifdef ATE_TXTHREAD
	pAd->ATECtrl.tx_thread[0].is_init = FALSE;
#endif
	pAd->ATECtrl.pkt_skb = NULL;
	return Status;
}


INT32 MtTestModeExit(RTMP_ADAPTER *pAd)
{
	UINT32 Status = 0;
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *AteOp = NULL;
	struct _ATE_IF_OPERATION *ATEIfOps = NULL;
	AteOp = ATECtrl->ATEOp;
	ATEIfOps = ATECtrl->ATEIfOps;
	os_free_mem(ATEIfOps);
	os_free_mem(AteOp);

#ifdef RTMP_PCI_SUPPORT

	if (IS_PCI_INF(pAd))
		RTMP_OS_EXIT_COMPLETION(&pAd->LbCtrl.LoopBackPCITxEvent);

#endif
	RTMP_OS_EXIT_COMPLETION(&pAd->LbCtrl.LoopBackEvent);
	NdisFreeSpinLock(&pAd->LbCtrl.LoopBackLock);
	return Status;
}


INT32 MtATECh2Freq(UINT32 Channel, UINT32 band_idx)
{
	UINT32 Freq = 0;

	switch (band_idx) {
	case 0:
		if (Channel >= 1 && Channel <= 13)
			Freq = 2407 + Channel * 5;
		else if (Channel == 14)
			Freq = 2484;

		break;

	case 1:
		if (Channel >= 7 && Channel <= 181)
			Freq = 5000 + Channel * 5;
		else if (Channel >= 184 && Channel <= 196)
			Freq = 4000 + Channel * 5;
		else if (Channel == 6)
			Freq = 5032;
		else if (Channel == 182)
			Freq = 4912;
		else if (Channel == 183)
			Freq = 4917;

		break;

	default:
		break;
	}

	return Freq;
}


INT32 MtATEGetTxPwrGroup(UINT32 Channel, UINT32 band_idx, UINT32 Ant_idx)
{
	UINT32 Group = 0;
	UINT32 Freq = MtATECh2Freq(Channel, band_idx);
	UINT32 i;
	UINT32 NumOfMap = (sizeof(txpwr_group_map) / sizeof(struct _ATE_TXPWR_GROUP_MAP));

	for (i = 0; i < NumOfMap; ++i) {
		if (Freq > txpwr_group_map[i].start && Freq <= txpwr_group_map[i].end) {
			Group = txpwr_group_map[i].group[Ant_idx];
			break;
		}
	}

	return Group;
}

INT32 MtATERSSIOffset(PRTMP_ADAPTER pAd, INT32 rssi_orig, UINT32 ant_idx, INT32 ch_band)
{
	UCHAR rssi_offset = 0;
	UINT32 *band_offset = NULL;
	UINT32 offset = 0;

	if (ch_band > eeprom_rssi_offset.n_band - 1)
		return rssi_orig;

	if (eeprom_rssi_offset.rssi_eeprom_band_offset[ch_band])
		band_offset = eeprom_rssi_offset.rssi_eeprom_band_offset[ch_band];
	else
		return rssi_orig;

	if ((ant_idx >
		 eeprom_rssi_offset.n_band_offset[ch_band] - 1))
		return rssi_orig;
	else if (band_offset[ant_idx] == 0)
		return rssi_orig;

	offset = band_offset[ant_idx];
	rssi_offset = pAd->EEPROMImage[offset];

	if (rssi_offset & (1 << 7) && rssi_offset != 0xFF) {
		if (rssi_offset & (1 << 6))
			return rssi_orig + (rssi_offset & 0x3f);
		else
			return rssi_orig - (rssi_offset & 0x3f);
	}

	return rssi_orig;
}
#ifdef PRE_CAL_MT7622_SUPPORT
INT MtATE_DPD_Cal_Store_Proc_7622(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 u4CalId;
	UINT8 i = 0;
	USHORT doCal1 = 0;
	MT_SWITCH_CHANNEL_CFG ch_cfg;

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Currently not in FLASH or BIN MODE,return.\n", __func__));
		return FALSE;
	}
	/* set channel command , per group calibration - set to channel 7, BW20 */
	ch_cfg.Bw = BW_20;
	ch_cfg.CentralChannel = 7;
	ch_cfg.TxStream = 4;
	ch_cfg.RxStream = 4;
	ch_cfg.ControlChannel = 7;
	ch_cfg.ControlChannel2 = 0;
	ch_cfg.BandIdx = 0;
	ch_cfg.bScan = 0;
	MtCmdChannelSwitch(pAd, ch_cfg);

	pAd->bPreCalMode = TRUE;
	/* Retest Recal - TXLPFG */
	u4CalId = TX_LPFG;
	MtCmdRfTestRecal(pAd, u4CalId, TX_LPFG_RESP_LEN);

	/* Retest Recal - TXDCIQ */
	u4CalId = TX_DCIQC;
	MtCmdRfTestRecal(pAd, u4CalId, TX_DCIQ_RESP_LEN);

	pAd->TxDpdCalOfst = 0;
	RTMPZeroMemory(pAd->CalTXDPDImage, CAL_TXDPD_SIZE);

	/* Retest Recal - TXDPD */
	for (i = 1; i <= 14; i++) {
		ch_cfg.CentralChannel = i;
		ch_cfg.ControlChannel = i;
		MtCmdChannelSwitch(pAd, ch_cfg);
		u4CalId = TX_DPD_LINK;
		MtCmdRfTestRecal(pAd, u4CalId, TX_DPD_LINK_RESP_LEN);
	}

	/* raise DoCalibrate bits */
	if (pAd->E2pAccessMode == E2P_BIN_MODE)
		rtmp_ee_bin_read16(pAd, 0x52, &doCal1);

#ifdef RTMP_FLASH_SUPPORT
	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		rtmp_ee_flash_read(pAd, 0x52, &doCal1);
#endif
	/* raise bit3 */
	doCal1 |= (1 << 3);
	if (pAd->E2pAccessMode == E2P_BIN_MODE) {
		rtmp_ee_bin_write16(pAd, 0x52, doCal1);
		rtmp_ee_write_to_bin(pAd);      /* writeback to eeprom file */
	}
#ifdef RTMP_FLASH_SUPPORT
	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		rtmp_ee_flash_write(pAd, 0x52, doCal1);
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("raised E2P 0x52 = %x\n", doCal1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("====================\n"));
	pAd->bPreCalMode = FALSE;
	return TRUE;
}
#endif /* PRE_CAL_MT7622_SUPPORT */

#ifdef PRE_CAL_TRX_SET1_SUPPORT
INT MtATE_DPD_Cal_Store_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG x = simple_strtol(arg, 0, 10);
	TXDPD_RESULT_T TxDpdResult;
	BOOLEAN toCR = FALSE; /* CR to Flash/Bin file */
	UINT16 CentralFreq = 0;
	UINT8 BW = 0;
	UINT8 i = 0;
	UINT8 j = 0;
	ULONG CalOffset = 0;
	USHORT doCal1 = 0;
	UINT8 RetryTimes = 5;
	BOOLEAN DPDPassOrFail = TRUE;
	BOOLEAN kABand = TRUE;
	BOOLEAN kGBand = TRUE;

	if (x == 0) { /* 2G */
		kABand = FALSE;
		kGBand = TRUE;
	} else if (x == 1) { /* 5G */
		kABand = TRUE;
		kGBand = FALSE;
	} else { /* all K */
		kABand = TRUE;
		kGBand = TRUE;
	}

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Currently not in FLASH or BIN MODE,return.\n", __func__));
		return FALSE;
	}
	/* Stop TX RX MAC */
	SetATE(pAd, "TXSTOP");
	SetATE(pAd, "RXSTOP");

	/* TXDPD ABand */
	if (kABand) {
		RTMPZeroMemory(pAd->CalDPDAPart1Image, TXDPD_IMAGE1_SIZE);

		/* TXDPD A20  */
		for (i = 0; i < DPD_A20_SIZE; i++) {
			BW = BW_20;
			CentralFreq = DPD_A20Freq[i];
			CalOffset = i * TXDPD_SIZE;

			for (j = 0; j < RetryTimes; j++) {
				RTMPZeroMemory(&TxDpdResult, sizeof(TXDPD_RESULT_T));
				MtCmdGetTXDPDCalResult(pAd, toCR, CentralFreq, BW, ABAND, FALSE, FALSE, &TxDpdResult);
				RtmpusecDelay(10);

				if (TxDpdResult.ResultSuccess)
					break;
			}

			if (TxDpdResult.ResultSuccess) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("==========TX A20 P1 freq %d save to flash offset %lx ========\n", CentralFreq, DPDPART1_OFFSET + CalOffset));
				ShowDPDData(pAd, TxDpdResult);
				memcpy(pAd->CalDPDAPart1Image + CalOffset, &TxDpdResult.u4DPDG0_WF0_Prim, TXDPD_SIZE);
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("!!!!!!  A20 freq %d TX still failed after %d retries !!!!!!!!\n", CentralFreq, RetryTimes));
				DPDPassOrFail = FALSE;
				goto exit;
			}
		}

#ifdef RTMP_FLASH_SUPPORT

		if (pAd->E2pAccessMode == E2P_FLASH_MODE)
			RtmpFlashWrite(pAd->hdev_ctrl, pAd->CalDPDAPart1Image,
				get_dev_eeprom_offset(pAd) + DPDPART1_OFFSET, TXDPD_PART1_LIMIT * TXDPD_SIZE);

#endif
		if (pAd->E2pAccessMode == E2P_BIN_MODE)
			rtmp_cal_write_to_bin(pAd, pAd->CalDPDAPart1Image, DPDPART1_OFFSET, TXDPD_PART1_LIMIT * TXDPD_SIZE);

		RtmpusecDelay(20000);
	}
	/* TXDPD G20 */
	if (kGBand) {
		RTMPZeroMemory(pAd->CalDPDAPart2Image, TXDPD_IMAGE2_SIZE);

		for (i = 0; i < DPD_G20_SIZE; i++) {
			BW = BW_20;
			CentralFreq = DPD_G20Freq[i];
			CalOffset = i * TXDPD_SIZE;

			for (j = 0; j < RetryTimes; j++) {
				RTMPZeroMemory(&TxDpdResult, sizeof(TXDPD_RESULT_T));
				MtCmdGetTXDPDCalResult(pAd, toCR, CentralFreq, BW, GBAND, FALSE, FALSE, &TxDpdResult);
				RtmpusecDelay(10);

				if (TxDpdResult.ResultSuccess)
					break;
			}

			if (TxDpdResult.ResultSuccess) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("==========TX G20 freq %d save to flash offset %lx ========\n", CentralFreq, DPDPART1_OFFSET + DPD_A20_SIZE * TXDPD_SIZE + CalOffset));
				ShowDPDData(pAd, TxDpdResult);
				memcpy(pAd->CalDPDAPart2Image + CalOffset, &TxDpdResult.u4DPDG0_WF0_Prim, TXDPD_SIZE);
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("!!!!!!  G20 freq %d TX still failed after %d retries !!!!!!!!\n", CentralFreq, RetryTimes));
				DPDPassOrFail = FALSE;
				goto exit;
			}
		}

#ifdef RTMP_FLASH_SUPPORT

		if (pAd->E2pAccessMode == E2P_FLASH_MODE)
			RtmpFlashWrite(pAd->hdev_ctrl, pAd->CalDPDAPart2Image,
				get_dev_eeprom_offset(pAd) + DPDPART2_OFFSET, (DPD_G20_SIZE * TXDPD_SIZE));

#endif

		if (pAd->E2pAccessMode == E2P_BIN_MODE)
			rtmp_cal_write_to_bin(pAd, pAd->CalDPDAPart2Image, DPDPART2_OFFSET, (DPD_G20_SIZE * TXDPD_SIZE));
	}

	/* raise DoCalibrate bits */
	if (pAd->E2pAccessMode == E2P_BIN_MODE)
		rtmp_ee_bin_read16(pAd, 0x52, &doCal1);

#ifdef RTMP_FLASH_SUPPORT

	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		rtmp_ee_flash_read(pAd, 0x52, &doCal1);

#endif
	doCal1 |= (1 << 0);
	/* raise bit 4 to denote 16 entry TXDPD */
	doCal1 |= (1 << 4);

	if (pAd->E2pAccessMode == E2P_BIN_MODE) {
		rtmp_ee_bin_write16(pAd, 0x52, doCal1);
		rtmp_ee_write_to_bin(pAd);		/* writeback to eeprom file */
	}

#ifdef RTMP_FLASH_SUPPORT

	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		rtmp_ee_flash_write(pAd, 0x52, doCal1);

#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("raised E2P 0x52 = %x\n", doCal1));

	/* reload test -- for debug only */
	if (pAd->KtoFlashDebug) {
		ULONG CalOffset = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\n######################### reload caldata for debug ####################################\n"));
		RtmpusecDelay(20000);

#ifdef RTMP_FLASH_SUPPORT

		if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
			RtmpFlashRead(pAd->hdev_ctrl, pAd->CalDPDAPart1Image,
				get_dev_eeprom_offset(pAd) + DPDPART1_OFFSET, TXDPD_IMAGE1_SIZE);
			RtmpFlashRead(pAd->hdev_ctrl, pAd->CalDPDAPart2Image,
				get_dev_eeprom_offset(pAd) + DPDPART2_OFFSET, TXDPD_IMAGE2_SIZE);
		}
#endif

		if (pAd->E2pAccessMode == E2P_BIN_MODE) {
			rtmp_cal_load_from_bin(pAd, pAd->CalDPDAPart1Image, DPDPART1_OFFSET, TXDPD_IMAGE1_SIZE);
			rtmp_cal_load_from_bin(pAd, pAd->CalDPDAPart2Image, DPDPART2_OFFSET, TXDPD_IMAGE2_SIZE);
		}

		/* Find flash offset base on CentralFreq */
		for (i = 0; i < DPD_ALL_SIZE; i++) {
			CalOffset = i * TXDPD_SIZE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: reload flash offset [%lx]  freq [%d]=================\n",
					  __func__, CalOffset + DPDPART1_OFFSET, DPD_AllFreq[i]));

			if (i < DPD_A20_SIZE)
				memcpy(&TxDpdResult.u4DPDG0_WF0_Prim, pAd->CalDPDAPart1Image + CalOffset, TXDPD_SIZE);
			else {
				CalOffset = (i - DPD_A20_SIZE) * TXDPD_SIZE;
				memcpy(&TxDpdResult.u4DPDG0_WF0_Prim, pAd->CalDPDAPart2Image + CalOffset, TXDPD_SIZE);
			}

			ShowDPDData(pAd, TxDpdResult);
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("raised E2P 0x52 = %x\n", doCal1));
exit:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("====================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("TX_SELF_TEST : [%s]\n",
			  (DPDPassOrFail == TRUE) ? "PASS" : "FAIL"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("====================\n"));
	return TRUE;
}


INT MtATE_DCOC_Cal_Store_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ULONG x = simple_strtol(arg, 0, 10);
	RXDCOC_RESULT_T RxDcocResult;
	BOOLEAN toCR = FALSE;
	UINT16 CentralFreq = 0;
	UINT8 BW = 0;
	UINT8 i = 0;
	UINT8 j = 0;
	ULONG CalOffset = 0;
	USHORT doCal1 = 0;
	UINT8 RetryTimes = 5;
	BOOLEAN DCOCPassOrFail = TRUE;
	BOOLEAN kABand = TRUE;
	BOOLEAN kGBand = TRUE;

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Currently not in FLASH or BIN MODE,return.\n",
				  __func__));
		return FALSE;
	}

	RTMPZeroMemory(pAd->CalDCOCImage, DCOC_IMAGE_SIZE);

	if (x == 0) { /* 2G */
		kABand = FALSE;
		kGBand = TRUE;
	} else if (x == 1) { /* 5G */
		kABand = TRUE;
		kGBand = FALSE;
	} else { /* all K */
		kABand = TRUE;
		kGBand = TRUE;
	}
	 /* Disable RMAC */
	MtATESetMacTxRx(pAd, ASIC_MAC_RX, FALSE, TESTMODE_BAND0);
	if (IS_ATE_DBDC(pAd))
		MtATESetMacTxRx(pAd, ASIC_MAC_RX, FALSE, TESTMODE_BAND1);

	/* RXDCOC ABand */
	if (kABand) {
		for (i = 0; i < K_A20_SIZE; i++) {
			BW = BW_20;
			CentralFreq = K_A20Freq[i];
			CalOffset = i * RXDCOC_SIZE;

			for (j = 0; j < RetryTimes; j++) {
				RTMPZeroMemory(&RxDcocResult, sizeof(RXDCOC_RESULT_T));
				MtCmdGetRXDCOCCalResult(pAd, toCR, CentralFreq, BW, ABAND, FALSE, FALSE, &RxDcocResult);
				RtmpusecDelay(10);

				if (RxDcocResult.ResultSuccess)
					break;
			}

			if (RxDcocResult.ResultSuccess) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("========== A20 freq %d save to offset %lx ========\n",
						  CentralFreq, DCOC_OFFSET + CalOffset));
				ShowDCOCData(pAd, RxDcocResult);
				memcpy(pAd->CalDCOCImage + CalOffset, &RxDcocResult.ucDCOCTBL_I_WF0_SX0_LNA[0], RXDCOC_SIZE);
			} else {
				ShowDCOCData(pAd, RxDcocResult);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("!!!!!!  A20 freq %d RX still failed after %d retries !!!!!!!!\n", CentralFreq, RetryTimes));
				DCOCPassOrFail = FALSE;
				goto exit;
			}
		}

		for (i = 0; i < K_A40_SIZE; i++) {
			BW = BW_40;
			CentralFreq = K_A40Freq[i];
			CalOffset = (K_A20_SIZE + i) * RXDCOC_SIZE;

			for (j = 0; j < RetryTimes; j++) {
				RTMPZeroMemory(&RxDcocResult, sizeof(RXDCOC_RESULT_T));
				MtCmdGetRXDCOCCalResult(pAd, toCR, CentralFreq,	BW, ABAND, FALSE, FALSE, &RxDcocResult);
				RtmpusecDelay(10);

				if (RxDcocResult.ResultSuccess)
					break;
			}

			if (RxDcocResult.ResultSuccess) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("========== A40 freq %d save to offset %lx ========\n",
						  CentralFreq, DCOC_OFFSET + CalOffset));
				ShowDCOCData(pAd, RxDcocResult);
				memcpy(pAd->CalDCOCImage + CalOffset, &RxDcocResult.ucDCOCTBL_I_WF0_SX0_LNA[0], RXDCOC_SIZE);
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("!!!!!!  A40 freq %d RX still failed after %d retries !!!!!!!!\n", CentralFreq, RetryTimes));
				DCOCPassOrFail = FALSE;
				goto exit;
			}
		}

		for (i = 0; i < K_A80_SIZE; i++) {
			BW = BW_80;
			CentralFreq = K_A80Freq[i];
			CalOffset = (K_A20_SIZE + K_A40_SIZE + i) * RXDCOC_SIZE;

			for (j = 0; j < RetryTimes; j++) {
				RTMPZeroMemory(&RxDcocResult, sizeof(RXDCOC_RESULT_T));
				MtCmdGetRXDCOCCalResult(pAd, toCR, CentralFreq,	BW, ABAND, FALSE, FALSE, &RxDcocResult);
				RtmpusecDelay(10);

				if (RxDcocResult.ResultSuccess)
					break;
			}

			if (RxDcocResult.ResultSuccess) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("========== A80 freq %d save to offset %lx ========\n", CentralFreq, DCOC_OFFSET + CalOffset));
				ShowDCOCData(pAd, RxDcocResult);
				memcpy(pAd->CalDCOCImage + CalOffset, &RxDcocResult.ucDCOCTBL_I_WF0_SX0_LNA[0], RXDCOC_SIZE);
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("!!!!!!  A80 freq %d RX still failed after %d retries !!!!!!!!\n", CentralFreq, RetryTimes));
				DCOCPassOrFail = FALSE;
				goto exit;
			}
		}
	}

	/* RXDCOC GBand */
	if (kGBand) {
		for (i = 0; i < K_G20_SIZE; i++) {
			BW = BW_20;
			CentralFreq = K_G20Freq[i];
			CalOffset = (K_A20_SIZE + K_A40_SIZE + K_A80_SIZE + i) * RXDCOC_SIZE;

			for (j = 0; j < RetryTimes; j++) {
				RTMPZeroMemory(&RxDcocResult, sizeof(RXDCOC_RESULT_T));
				MtCmdGetRXDCOCCalResult(pAd, toCR, CentralFreq, BW, GBAND, FALSE, FALSE, &RxDcocResult);
				RtmpusecDelay(10);

				if (RxDcocResult.ResultSuccess)
					break;
			}

			if (RxDcocResult.ResultSuccess) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("========== G20 freq %d save to offset %lx ========\n", CentralFreq, DCOC_OFFSET + CalOffset));
				ShowDCOCData(pAd, RxDcocResult);
				memcpy(pAd->CalDCOCImage + CalOffset, &RxDcocResult.ucDCOCTBL_I_WF0_SX0_LNA[0],	RXDCOC_SIZE);
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("!!!!!!  G20 freq %d RX still failed after %d retries !!!!!!!!\n", CentralFreq, RetryTimes));
				DCOCPassOrFail = FALSE;
				goto exit;
			}
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("========== save to offset %x size %d========\n",
#ifdef RTMP_FLASH_SUPPORT
			  get_dev_eeprom_offset(pAd) +
#endif
			  DCOC_OFFSET, (K_ALL_SIZE * RXDCOC_SIZE)));
#ifdef RTMP_FLASH_SUPPORT

	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		RtmpFlashWrite(pAd->hdev_ctrl, pAd->CalDCOCImage,
			get_dev_eeprom_offset(pAd) + DCOC_OFFSET, (K_ALL_SIZE * RXDCOC_SIZE));

#endif

	if (pAd->E2pAccessMode == E2P_BIN_MODE)
		rtmp_cal_write_to_bin(pAd, pAd->CalDCOCImage, DCOC_OFFSET, (K_ALL_SIZE * RXDCOC_SIZE));

	/* raise DoCalibrate bits */
#ifdef RTMP_FLASH_SUPPORT

	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		rtmp_ee_flash_read(pAd, 0x52, &doCal1);

#endif

	if (pAd->E2pAccessMode == E2P_BIN_MODE)
		rtmp_ee_bin_read16(pAd, 0x52, &doCal1);

	doCal1 |= (1 << 1);
#ifdef RTMP_FLASH_SUPPORT

	if (pAd->E2pAccessMode == E2P_FLASH_MODE)
		rtmp_ee_flash_write(pAd, 0x52, doCal1);

#endif

	if (pAd->E2pAccessMode == E2P_BIN_MODE) {
		rtmp_ee_bin_write16(pAd, 0x52, doCal1);
		rtmp_ee_write_to_bin(
			pAd);		/* XXX: remember to writeback modified eeprom to file */
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("raised E2P 0x52 = %x\n", doCal1));

	if (pAd->KtoFlashDebug) {
		ULONG CalOffset = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("######################### reload caldata for debug ####################################\n"));
		RtmpusecDelay(20000);
#ifdef RTMP_FLASH_SUPPORT

		if (pAd->E2pAccessMode == E2P_FLASH_MODE)
			RtmpFlashRead(pAd->hdev_ctrl, pAd->CalDCOCImage,
				get_dev_eeprom_offset(pAd) + DCOC_OFFSET, (K_ALL_SIZE * RXDCOC_SIZE));

#endif

		if (pAd->E2pAccessMode == E2P_BIN_MODE)
			rtmp_cal_load_from_bin(pAd, pAd->CalDCOCImage, DCOC_OFFSET, (K_ALL_SIZE * RXDCOC_SIZE));

		/* Find offset base on CentralFreq */
		for (i = 0; i < K_ALL_SIZE; i++) {
			CalOffset = i * RXDCOC_SIZE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: reload from offset [%lx]  freq [%d]=================\n",
					  __func__, CalOffset + DCOC_OFFSET, K_AllFreq[i]));
			memcpy(&RxDcocResult.ucDCOCTBL_I_WF0_SX0_LNA[0],
				   pAd->CalDCOCImage + CalOffset, RXDCOC_SIZE);
			ShowDCOCData(pAd, RxDcocResult);
		}
	}

exit:
	/* Enable RMAC */
	MtATESetMacTxRx(pAd, ASIC_MAC_RX, TRUE, TESTMODE_BAND0);
	if (IS_ATE_DBDC(pAd))
		MtATESetMacTxRx(pAd, ASIC_MAC_RX, TRUE, TESTMODE_BAND1);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("====================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("RX_SELF_TEST : [%s]\n",
			  (DCOCPassOrFail == TRUE) ? "PASS" : "FAIL"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("====================\n"));
	return TRUE;
}
#endif /* PRE_CAL_TRX_SET1_SUPPORT */

#ifdef PRE_CAL_TRX_SET2_SUPPORT
INT MtATE_Pre_Cal_Proc(RTMP_ADAPTER *pAd, UINT8 CalId, UINT32 ChGrpId)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: Cal Id = %d, ChGrpId = %d\n", __func__, CalId, ChGrpId));
	/* Initialization */
	pAd->PreCalWriteOffSet = 0;
	pAd->ChGrpMap = PreCalGroupList[ChGrpId];

	if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
		pAd->PreCalStoreBuffer = pAd->EEPROMImage + PRECALPART_OFFSET;
	} else {
		ret = os_alloc_mem(pAd, &pAd->PreCalStoreBuffer, PRE_CAL_SIZE);

		if (ret != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("\x1b[42m %s : Not enough memory for pre-cal stored buffer!!\x1b[m\n", __func__));
		}
	}

	if (ret == NDIS_STATUS_SUCCESS)
		MtCmdGetPreCalResult(pAd, PreCalItemList[CalId], PreCalGroupList[ChGrpId]);

	return TRUE;
}
#endif/* PRE_CAL_TRX_SET2_SUPPORT */

INT32 MtATETssiTrainingProc(RTMP_ADAPTER *pAd, UCHAR ucBW, UCHAR ucBandIdx)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 ret = 0;

	/* only applu TSSI Training for MT7615 */
	if (IS_MT7615(pAd)) {
		if (ucBW == BW_8080 || ucBW == BW_160) {
			/* Check iPA or ePA status */
			MtEPAcheck(pAd);

			/* TSSI Training only for IPA case */
			if (!pAd->fgEPA) {
				UCHAR PerPktBW, PhyMode, Mcs, Nss;
				UINT32 TxLength;

				/* Backup parameters for Phymode, BW, Rate, VHTNss, TxLength */
				PerPktBW = TESTMODE_GET_PARAM(ATECtrl, ucBandIdx, PerPktBW);
				PhyMode  = TESTMODE_GET_PARAM(ATECtrl, ucBandIdx, PhyMode);
				Mcs 	 = TESTMODE_GET_PARAM(ATECtrl, ucBandIdx, Mcs);
				Nss 	 = TESTMODE_GET_PARAM(ATECtrl, ucBandIdx, Nss);
				TxLength = TESTMODE_GET_PARAM(ATECtrl, ucBandIdx, TxLength);

				/* Config TSSI Tracking enable */
				MtATETSSITracking(pAd, TRUE);
				/* Config FCBW ON */
				MtATEFCBWCfg(pAd, TRUE);
				/* Config DBW 80MHz */
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, PerPktBW, BW_80);
				/* Config VHT mode */
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, PhyMode, MODE_VHT);
				/* Config MCS rate */
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, Mcs, MCS_9);
				/* Config 4 Nss */
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, Nss, 4);
				/* Config Tx packet length */
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, TxLength, 100);
				/* Start Tx for 25ms */
				ATEOp->StartTx(pAd);
				RtmpOsMsDelay(25);
				/* Stop Tx */
				ATEOp->StopTx(pAd);
				/* Save compensation value to Global variable (FCBW on case) */
				MtTSSICompBackup(pAd, TRUE);
				/* Config FCBW OFF */
				MtATEFCBWCfg(pAd, FALSE);
				/* Config DBW 160MHz */
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, PerPktBW, BW_160);
				/* Config VHT mode */
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, PhyMode, MODE_VHT);
				/* Config MCS rate */
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, Mcs, MCS_9);
				/* Config 2 Nss */
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, Nss, 2);
				/* Config Tx packet length */
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, TxLength, 100);
				/* Start Tx for 25ms */
				ATEOp->StartTx(pAd);
				RtmpOsMsDelay(25);
				/* Stop Tx */
				ATEOp->StopTx(pAd);
				/* Save compensation value to Global variable (FCBW off case) */
				MtTSSICompBackup(pAd, FALSE);
				/* Config Compensation CR */
				MtTSSICompCfg(pAd);
				/* Config TSSI Tracking disable */
				MtATETSSITracking(pAd, FALSE);
				/* Config FCBW ON */
				MtATEFCBWCfg(pAd, TRUE);

				/* Resotre paratemters for Phymode, BW, Rate, VHTNss, TxLength */
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, PerPktBW, PerPktBW);
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, PhyMode, PhyMode);
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, Mcs, Mcs);
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, Nss, Nss);
				TESTMODE_SET_PARAM(ATECtrl, ucBandIdx, TxLength, TxLength);

				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TSSI Training Done!!\n", __func__));
			}
		}
	}

	return ret;
}
