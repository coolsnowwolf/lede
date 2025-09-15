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
	mt_tx_pwr.h
*/
#ifndef __MT_TX_PWR_H__
#define __MT_TX_PWR_H__

struct MT_TX_PWR_CAP {
#define INTERNAL_PA 0
#define EXTERNAL_PA 1
	UINT8 pa_type;
#define TSSI_TRIGGER_STAGE 0
#define TSSI_COMP_STAGE 1
#define TSSI_CAL_STAGE 2
	UINT8 tssi_stage;
#define TSSI_0_SLOPE_G_BAND_DEFAULT_VALUE 0x84
#define TSSI_1_SLOPE_G_BAND_DEFAULT_VALUE 0x83
	UINT8 tssi_0_slope_g_band;
	UINT8 tssi_1_slope_g_band;
#define TSSI_0_OFFSET_G_BAND_DEFAULT_VALUE 0x0A
#define TSSI_1_OFFSET_G_BAND_DEFAULT_VALUE 0x0B
	UINT8 tssi_0_offset_g_band;
	UINT8 tssi_1_offset_g_band;
#define TX_TARGET_PWR_DEFAULT_VALUE 0x26
	CHAR tx_0_target_pwr_g_band;
	CHAR tx_1_target_pwr_g_band;
	CHAR tx_0_chl_pwr_delta_g_band[3];
	CHAR tx_1_chl_pwr_delta_g_band[3];
	CHAR delta_tx_pwr_bw40_g_band;

	CHAR tx_pwr_cck_1_2;
	CHAR tx_pwr_cck_5_11;
	CHAR tx_pwr_g_band_ofdm_6_9;
	CHAR tx_pwr_g_band_ofdm_12_18;
	CHAR tx_pwr_g_band_ofdm_24_36;
	CHAR tx_pwr_g_band_ofdm_48;
	CHAR tx_pwr_g_band_ofdm_54;
	CHAR tx_pwr_ht_bpsk_mcs_0_8;
	CHAR tx_pwr_ht_bpsk_mcs_32;
	CHAR tx_pwr_ht_qpsk_mcs_1_2_9_10;
	CHAR tx_pwr_ht_16qam_mcs_3_4_11_12;
	CHAR tx_pwr_ht_64qam_mcs_5_13;
	CHAR tx_pwr_ht_64qam_mcs_6_14;
	CHAR tx_pwr_ht_64qam_mcs_7_15;
};


#endif
