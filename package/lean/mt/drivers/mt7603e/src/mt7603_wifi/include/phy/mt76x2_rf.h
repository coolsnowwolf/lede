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
	mt76x2_rf.h
*/


#ifndef __MT76x2_RF_H__
#define __MT76x2_RF_H__

enum {
	CHL_GRP1 = 0x01,
	CHL_GRP2,
	CHL_GRP3,
	CHL_GRP4,
	CHL_GRP5,
	CHL_GRP6,
	CHL_GRP7,
	CHL_GRP_MAX = CHL_GRP7,
};

#define RFDIGI_TOP0 0x000
#define RFDIGI_TOP2 0x008
#define RFDIGI_TOP3 0x00C
#define RFDIGI_TOP4 0x010
#define A_BAND_IQM_TSSI_DIV_LPF 0x054
#define A_BAND_PA 0x058
#define RFDIGI_ABB_TO_AFE5 0x114
#define RF_ABB 0x130
#define RFDIGI_TO_SX2 0x144
#define SX2_FCAL_TARGET_MASK (0x3fff << 11)
#define SX2_FCAL_TARGET(p) (((p) & 0x3fff) << 11)
#define RFDIGI_TO_SX5 0x150
#define RFDIGI_TO_SX7 0x158
#define SX7_INTCS_MASK (0x3ff)
#define SX7_INTCS(p) (((p) & 0x3ff))
#define SX7_FRACES_MASK (0xfffff << 12)
#define SX7_FRACES(p) (((p) & 0xfffff) << 12)
#define SX5_REPA_NRBIAS_2_0_MASK (0x7 << 9)
#define SX5_REPA_NRBIAS_2_0(p) (((p) & 0x07) << 9)
#define RFDIGI_TRX0 0x200
#define TRX0_SW_TXIQ_CANCEL_EN (0x1 << 10)
#define RFDIGI_TRX4 0x210
#define RFDIGI_TRX17 0x244
#define RFDIGI_TRX21 0x254
#define TRX21_LNA_OUTCAP_A_HG_MASK (0x07 << 0)
#define TRX21_LNA_OUTCAP_A_HG(p) (((p) & 0x07) << 0)
#define TRX21_LNA_OUTCAP_A_MG_MASK (0x07 << 4)
#define TRX21_LNA_OUTCAP_A_MG(p) (((p) & 0x07) << 4)
#define TRX21_LNA_OUTCAP_A_LG_MASK (0x07 << 8)
#define TRX21_LNA_OUTCAP_A_LG(p) (((p) & 0x07) << 8)
#define TRX21_LNA_OUTCAP_A_ULG_MASK (0x07 << 12)
#define TRX21_LNA_OUTCAP_A_ULG(p) (((p) & 0x07) << 12)
#define TRX21_TZ_GCR_A_HG_MASK (0x07 << 16)
#define TRX21_TZ_GCR_A_HG(p) (((p) & 0x07) << 16)
#define TRX21_TZ_GCR_A_MG_MASK (0x07 << 20)
#define TRX21_TZ_GCR_A_MG(p) (((p) & 0x07) << 20)
#define TRX21_TZ_GCR_A_LG_MASK (0x07 << 24)
#define TRX21_TZ_GCR_A_LG(p) (((p) & 0x07) << 24)
#define TRX21_TZ_GCR_A_ULG_MASK (0x07 << 28)
#define TRX21_TZ_GCR_A_ULG(p) (((p) & 0x07) << 28)
#define RFDIGI_TRX24 0x260
#define TRX24_GM_GC_A_HG_MASK (0x0f << 0)
#define TRX24_GM_GC_A_HG(p) (((p) & 0x0f) << 0)
#define TRX24_GM_GC_A_MG_MASK (0x0f << 4)
#define TRX24_GM_GC_A_MG(p) (((p) & 0x0f) << 4)
#define TRX24_GM_GC_A_LG_MASK (0x0f << 8)
#define TRX24_GM_GC_A_LG(p) (((p) & 0x0f) << 8)
#define TRX24_GM_GC_A_ULG_MASK (0x0f << 12)
#define TRX24_GM_GC_A_ULG(p) (((p) & 0x0f) << 12)
#define TRX24_TZ_FBC_A_HG_MASK (0x0f << 16)
#define TRX24_TZ_FBC_A_HG(p) (((p) & 0x0f) << 16)
#define TRX24_TZ_FBC_A_MG_MASK (0x0f << 20)
#define TRX24_TZ_FBC_A_MG(p) (((p) & 0x0f) << 20)
#define TRX24_TZ_FBC_A_LG_MASK (0x0f << 24)
#define TRX24_TZ_FBC_A_LG(p) (((p) & 0x0f) << 24)
#define TRX24_TZ_FBC_A_ULG_MASK (0x0f << 28)
#define TRX24_TZ_FBC_A_ULG(p) (((p) & 0x0f) << 28)
#define RFDIGI_TRX27 0x26C
#define RFDIGI_TRX28 0x270
#define RFDIGI_TRX29 0x274
#define RFDIGI_TRX38 0x298
#define RFDIGI_TRX39 0x29C
#define RFDIGI_TRX42 0x2A8
#define RFDIGI_SX7 0x31C
#define RFDIGI_SX8 0x320


#define W_RFDATA 0x0524
#define R_RFDATA 0x052C
#define RF_CTRL 0x0528
#define RF_ADDR_MASK 0xfff
#define RF_ADDR(p) ((p) & 0xfff)
#define RF_R_W_CTRL (1 << 12)
#define RF_READY_MASK (1<< 13)
#define RF_READY(p) (((p) & RF_READY_MASK) == (0x0 << 13))
#define RF_IDX_MASK (1 << 16)
#define RF_IDX(p) (((p) & 0x1) << 16)

typedef struct _MT_RF_REG {
	UINT8 rf_idx;
	UINT16 offset;	
	UINT32 data;
} MT_RF_REG, *PMT_RF_REG;

typedef struct _CHL_GRP_RF_REG {
	UINT8 chl_grp;
	UINT16 chl_start;
	UINT16 chl_end;
	UINT8 lna_outcap_a_hg;
	UINT8 lna_outcap_a_mg;
	UINT8 lna_outcap_a_lg;
	UINT8 lna_outcap_a_ulg;
	UINT8 tz_gcr_a_hg;
	UINT8 tz_gcr_a_mg;
	UINT8 tz_gcr_a_lg;
	UINT8 tz_gcr_a_ulg;
	UINT8 tz_fbc_a_hg;
	UINT8 tz_fbc_a_mg;
	UINT8 tz_fbc_a_lg;
	UINT8 tz_fbc_a_ulg;
	UINT8 gm_gc_a_hg;
	UINT8 gm_gc_a_mg;
	UINT8 gm_gc_a_lg;
	UINT8 gm_gc_a_ulg;
} CHL_GRP_RF_REG, *PCHL_GRP_RF_REG;

INT mt76x2_rf_write(struct _RTMP_ADAPTER *pAd, UINT8 rf_idx, UINT16 offset, UINT32 data);
INT mt76x2_rf_read(struct _RTMP_ADAPTER *pAd, UINT8 rf_idx, UINT16 offset, UINT32 *data);
INT32 MT76x2ShowAllRF(struct _RTMP_ADAPTER *pAd);
INT32 MT76x2ShowPartialRF(struct _RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End);

#endif
