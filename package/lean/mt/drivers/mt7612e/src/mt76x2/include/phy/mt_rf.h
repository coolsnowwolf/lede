#ifndef __MT_RF_H__
#define __MT_RF_H__

struct _RTMP_ADAPTER;

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
#define RFDIGI_TOP1 0x004
#define RFDIGI_TOP2 0x008
#define RFDIGI_TOP3 0x00C
#define RFDIGI_TOP4 0x010
#define RG_WF0_RXG_TOP 0x44
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
#define RFDIGI_TRX0  0x200
#define TRX0_SW_TXIQ_CANCEL_EN (0x1 << 10)
#define RFDIGI_TRX4  0x210
#define RFDIGI_TRX5  0x214
#define RFDIGI_TRX7  0x21C
#define RFDIGI_TRX8  0x220
#define RFDIGI_TRX9  0x224
#define RFDIGI_TRX10 0x228
#define RFDIGI_TRX17 0x244
#define RFDIGI_TRX19 0x24C
#define RFDIGI_TRX20 0x250
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
#define RFDIGI_TRX25 0x264
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
#define RFDIGI_TRX30 0x278
#define RFDIGI_TRX38 0x298
#define RFDIGI_TRX39 0x29C
#define RFDIGI_TRX42 0x2A8
#define RFDIGI_SX7   0x31C
#define RFDIGI_SX8   0x320
#define RFDIGI_SX10  0x328
#define RFDIGI_SX13  0x334
#define RFDIGI_ID    0xFFF


typedef struct _MT_RF_REG {
	u8 rf_idx;
	u16 offset;	
	u32 data;
} MT_RF_REG, *PMT_RF_REG;

typedef struct _CHL_GRP_RF_REG {
	u8 chl_grp;
	u16 chl_start;
	u16 chl_end;
	u8 lna_outcap_a_hg;
	u8 lna_outcap_a_mg;
	u8 lna_outcap_a_lg;
	u8 lna_outcap_a_ulg;
	u8 tz_gcr_a_hg;
	u8 tz_gcr_a_mg;
	u8 tz_gcr_a_lg;
	u8 tz_gcr_a_ulg;
	u8 tz_fbc_a_hg;
	u8 tz_fbc_a_mg;
	u8 tz_fbc_a_lg;
	u8 tz_fbc_a_ulg;
	u8 gm_gc_a_hg;
	u8 gm_gc_a_mg;
	u8 gm_gc_a_lg;
	u8 gm_gc_a_ulg;
} CHL_GRP_RF_REG, *PCHL_GRP_RF_REG;

int mt_rf_write(
	struct _RTMP_ADAPTER *ad,
	u8 rf_idx,
	u16 offset,
	u32 data);

int mt_rf_read(
	struct _RTMP_ADAPTER *ad,
	u8 rf_idx,
	u16 offset,
	u32 *data);

#endif /* __MT_RF_H__ */
