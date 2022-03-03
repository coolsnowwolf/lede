#ifndef __MT76X2_H__
#define __MT76X2_H__

#include "../mcu/mcu_and.h"
#include "../phy/mt_rf.h"

struct _RTMP_ADAPTER;

#define MAX_RF_ID	127
#define MAC_RF_BANK 7

void mt76x2_init(struct _RTMP_ADAPTER *ad);
void mt76x2_adjust_per_rate_pwr_delta(struct _RTMP_ADAPTER *ad, u8 channel, char delta_pwr);
void mt76x2_get_tx_pwr_per_rate(struct _RTMP_ADAPTER *ad);
void mt76x2_antenna_sel_ctl(struct _RTMP_ADAPTER *ad);
int mt76x2_read_chl_pwr(struct _RTMP_ADAPTER *ad);
void mt76x2_pwrOn(struct _RTMP_ADAPTER *ad);
void mt76x2_calibration(struct _RTMP_ADAPTER *ad, u8 channel);
void mt76x2_external_pa_rf_dac_control(struct _RTMP_ADAPTER *ad, u8 channel);
void mt76x2_tssi_calibration(struct _RTMP_ADAPTER *ad, u8 channel);
void mt76x2_tssi_compensation(struct _RTMP_ADAPTER *ad, u8 channel);

int mt76x2_reinit_agc_gain(struct _RTMP_ADAPTER *ad, u8 channel);
int mt76x2_reinit_hi_lna_gain(struct _RTMP_ADAPTER *ad, u8 channel);
void mt76x2_get_external_lna_gain(struct _RTMP_ADAPTER *ad);
void mt76x2_get_agc_gain(struct _RTMP_ADAPTER *ad, BOOLEAN init_phase);
int get_chl_grp(u8 channel);
VOID mt76x2_ePA_per_rate_compensate_init(struct _RTMP_ADAPTER *pAd , BOOLEAN is_ePA);


int get_low_mid_hi_index(u8 channel);
void mt76x2_tx_pwr_gain(struct _RTMP_ADAPTER *ad, u8 channel, u8 bw);

void percentage_delta_pwr(struct _RTMP_ADAPTER *ad);

void mt76x2_update_tx_power_percentage(struct _RTMP_ADAPTER *ad);

void mt76x2_get_current_temp(struct _RTMP_ADAPTER *ad);
void mt76x2_read_temp_info_from_eeprom(struct _RTMP_ADAPTER *ad);
#ifdef RTMP_TEMPERATURE_TX_ALC
void mt76x2_read_tx_alc_info_from_eeprom(struct _RTMP_ADAPTER *ad);
void mt76x2_temp_tx_alc(struct _RTMP_ADAPTER *ad);
#endif /* RTMP_TEMPERATURE_TX_ALC */

#ifdef SINGLE_SKU_V2
void mt76x2_single_sku(struct _RTMP_ADAPTER *ad, u8 channel);
void mt76x2_read_single_sku_info_from_eeprom(struct _RTMP_ADAPTER *ad);
void mt76x2_make_up_rate_pwr_table(struct _RTMP_ADAPTER *ad);
UCHAR mt76x2_get_sku_channel_base_pwr(struct _RTMP_ADAPTER *ad, u8 channel);
void mt76x2_update_per_rate_pwr(struct _RTMP_ADAPTER *ad);
UCHAR mt76x2_update_sku_pwr(struct _RTMP_ADAPTER *ad, u8 channel);
INT32 mt76x2_sku_calculate_TxPwrAdj(struct _RTMP_ADAPTER *ad,struct _TXWI_NMAC *txwi_n);

#endif /* SINGLE_SKU_V2 */
VOID mt76x2_ePA_per_rate_compensate_init(struct _RTMP_ADAPTER *pAd , BOOLEAN is_ePA);

#ifdef ED_MONITOR
void mt7612_set_ed_cca(struct _RTMP_ADAPTER *ad, BOOLEAN enable);
#endif /* ED_MONITOR */

#ifdef RALINK_ATE
VOID mt76x2_ate_do_calibration(
	struct _RTMP_ADAPTER *ad, UINT32 cal_id, UINT32 param);
VOID mt76x2_ate_SendNullFrame(struct _RTMP_ADAPTER *ad);
#endif /* RALINK_ATE */

struct mt76x2_frequency_item {
	u8 channel;
	u32 fcal_target;
	u32 sdm_integer;
	u32 sdm_fraction;	
};

typedef struct _MT76x2_RATE_PWR_ITEM {
	CHAR mcs_pwr;
	INT32 sku_pwr;
} MT76x2_RATE_PWR_ITEM, *PMT76x2_RATE_PWR_ITEM;

typedef struct _MT76x2_RATE_PWR_TABLE {
	MT76x2_RATE_PWR_ITEM CCK[4];
	MT76x2_RATE_PWR_ITEM OFDM[8];
	MT76x2_RATE_PWR_ITEM HT[16];
	MT76x2_RATE_PWR_ITEM HT20[16]; // only for storing HT20 sku_pwr in BBPcurrentBW==80/40 case
	MT76x2_RATE_PWR_ITEM HT40[16]; // only for storing HT40 sku_pwr in BBPcurrentBW==80/40 case
	MT76x2_RATE_PWR_ITEM VHT1SS[10];
	MT76x2_RATE_PWR_ITEM VHT2SS[10];
	MT76x2_RATE_PWR_ITEM STBC[10];
	MT76x2_RATE_PWR_ITEM MCS32;
} MT76x2_RATE_PWR_Table, *PMT76x2_RATE_PWR_Table;

#ifdef CONFIG_CALIBRATION_COLLECTION
#define MAX_MT76x2_CALIBRATION_ID (RXIQC_FD_CALIBRATION_7662+1)
#define MAX_RECORD_REG	50
enum reg_rf0_addr {
	RF0_214 = 0,
	RF0_218 = 1,
	RF0_21C = 2,
	RF0_220 = 3,
	RF0_224 = 4,
	RF0_228 = 5,
	RF0_24C = 6,
	RF0_250 = 7,
	RF0_264 = 8,
	RF0_278 = 9,
};
enum reg_rf1_addr {
	RF1_214 = 20,
	RF1_218 = 21,
	RF1_21C = 22,
	RF1_220 = 23,
	RF1_224 = 24,
	RF1_228 = 25,
	RF1_24C = 26,
	RF1_250 = 27,
	RF1_264 = 28,
	RF1_278 = 29,
};

enum reg_bbp_addr {
	BBP_2774 = 0,
	BBP_2780 = 1,
	BBP_2784 = 2,
	BBP_2788 = 3,
	BBP_278C = 4,
	BBP_2790 = 5,
	BBP_2794 = 6,
	BBP_2798 = 7,
	BBP_279C = 8,
	BBP_27A0 = 9,
	BBP_27A4 = 10,
	BBP_27A8 = 11,
	BBP_27AC = 12,
	BBP_27B0 = 13,
	BBP_27B4 = 14,
	BBP_27B8 = 15,
	BBP_27BC = 16,
	BBP_27C0 = 17,
	BBP_27C4 = 18,
	BBP_27C8 = 19,
	BBP_27CC = 20,
	BBP_208C = 21,
	BBP_2720 = 22,
	BBP_2C60 = 23,
	BBP_2C64 = 24,
	BBP_2C70 = 25,
	BBP_2C74 = 26,
	BBP_2818 = 27,
	BBP_281C = 28,
	BBP_2820 = 29,
	BBP_2824 = 30,
	BBP_2828 = 31,
	BBP_282C = 32,
};

struct mt76x2_calibration_info {
	UINT16 	addr[MAX_RECORD_REG];
	u32 	value[MAX_RECORD_REG];
};

void record_calibration_info(struct _RTMP_ADAPTER *ad, u32 cal_id);
void dump_calibration_info(struct _RTMP_ADAPTER *ad, u32 cal_id);
#endif


#ifdef DMA_BUSY_RESET
#ifdef RTMP_PCI_SUPPORT
VOID WlanResetB(struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_PCI_SUPPORT */
#endif /* DMA_BUSY_RESET */
#endif
