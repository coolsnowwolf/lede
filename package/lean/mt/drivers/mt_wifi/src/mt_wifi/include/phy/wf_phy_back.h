/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * (c) Copyright 2013, MediaTek, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    wf_phy_back.h

    Abstract:
    Unify MAC Wireless Chip related PHY CRs definition & structures

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    mtk05208    20131102      Start coding for TxV, RxV, CR_PHYMUX_11
*/


/* Ref. wf_phycr_back.doc */
#ifndef __WF_PHY_BACK_H__
#define __WF_PHY_BACK_H__


/*#define WF_PHY_BASE     0x00010000 */

/* ??? */
/*#define CR_PHYMUX_11    (WF_PHY_BASE + 0x422c) */
typedef union _CR_PHYMUX_11_STRUCT {
	struct {
	UINT32 rsv_0:16;
	UINT32 cr_phymux_num_rx:2;
	UINT32 rsv_1:2;
	UINT32 cr_phymux_rx1_on:1;
	UINT32 cr_phymux_rx0_on:1;
	UINT32 rsv_2:4;
	UINT32 cr_phymux_tx1_on:1;
	UINT32 cr_phymux_tx0_on:1;
	UINT32 rsv_3:1;
	UINT32 cr_phymux_tx_ant_man:1;
	} field;
	UINT32 word;
} CR_PHYMUX_11_STRUCT;


/* Tx Vector */
#define CR_PHYMUX_12    (WF_PHY_BASE + 0x4230)
#define CR_PHYMUX_13    (WF_PHY_BASE + 0x4234)
#define CR_PHYMUX_14    (WF_PHY_BASE + 0x4238)
#define CR_PHYMUX_15    (WF_PHY_BASE + 0x423c)
#define CR_PHYMUX_16    (WF_PHY_BASE + 0x4240)
#define CR_PHYMUX_17    (WF_PHY_BASE + 0x4244)

#define TXV1    CR_PHYMUX_12
#define TXV2    CR_PHYMUX_13
#define TXV3    CR_PHYMUX_14
#define TXV4    CR_PHYMUX_15
#define TXV5    CR_PHYMUX_16
#define TXV6    CR_PHYMUX_17


/* Rx Vector */
#define CR_PHYMUX_18    (WF_PHY_BASE + 0x4248)
#define CR_PHYMUX_19    (WF_PHY_BASE + 0x425c)
#define CR_PHYMUX_20    (WF_PHY_BASE + 0x4250)
#define CR_PHYMUX_21    (WF_PHY_BASE + 0x4254)
#define CR_PHYMUX_22    (WF_PHY_BASE + 0x4258)
#define CR_PHYMUX_23    (WF_PHY_BASE + 0x425c)

#define RXV1    CR_PHYMUX_18
typedef union _RO_RXV1_STRUCT {
	struct {
#define LEGACY_CCK_MSK      0x7
#define LEGACY_OFDM_MSK     0xf
	UINT32 tx_rate:7;        /*[0..6] */
	UINT32 ht_stbc:2;        /*[7..8] */
#define BCC     0x0
#define LDPC    0x1
	UINT32 ht_adcode:1;      /*[9] */
	UINT32 ht_ext_ltf:2;     /*[10..11] */
#define LEGACY_CCK_MODE     0x0
#define LEGACY_OFDM_MODE    0x1
#define MIXED_MODE          0x2
#define GREEN_MODE          0x3
#define VHT_MODE            0x4
	UINT32 tx_mode:3;        /*[12..14] */
#define FEQ_20M_MODE        0x0
#define FEQ_40M_MODE        0x1
#define FEQ_80M_MODE        0x2
#define FEQ_160M_MODE       0x3
	UINT32 fr_mode:2;        /*[15..16] */
	UINT32 vht_a1_b2:1;      /*[17] */
	UINT32 ht_aggregation:1; /*[18] */
	UINT32 ht_short_gi:1;    /*[19]*/
	UINT32 ht_smooth:1;      /*[20]*/
	UINT32 ht_no_sound:1;    /*[21]*/
	UINT32 vhta2_b8_b1:8;    /*[22..29]*/
	UINT32 vhta1_b5_b4:2;    /*[30..31]*/
	} field;
	UINT32 word;
} RO_RXV1_STRUCT;

#define RXV2    CR_PHYMUX_19
typedef union _RO_RXV2_STRUCT {
	struct {
#define LEGACY_RX_LEN_MSK  0x00000fff
#define HT_RX_LEN_MSK      0x0000ffff
#define VHT_RX_LEN_MSK     0x001fffff
	UINT32 length:21;
	UINT32 vhta1_b16_b6:11;
	} field;
	UINT32 word;
} RO_RXV2_STRUCT;

#define RXV3    CR_PHYMUX_20
typedef union _RO_RXV3_STRUCT {
	struct {
	UINT32 vhta1_b21_b17:5;
	UINT32 ofdm_freq_trans_detect:1;
	UINT32 aci_detect:1;
	UINT32 sel_antenna:1;
	UINT32 rcpi0:8;
	UINT32 fagc0_eq_cal:1;
	UINT32 fagc0_cal_gain:3;
	UINT32 rcpi1:8;
	UINT32 fagc1_eq_cal:1;
	UINT32 fagc1_cal_gain:3;
	} field;
	UINT32 word;
} RO_RXV3_STRUCT;

#define RXV4    CR_PHYMUX_21
typedef union _RO_RXV4_STRUCT {
	struct {
	UINT32 ib_rssi:8;
	UINT32 wb_rssi:8;
	UINT32 fagc_lpf_gain:4;
	UINT32 rcpi2:8;
	UINT32 fagc2_eq_cal:1;
	UINT32 fagc2_cal_gain:3;
	} field;
	UINT32 word;
} RO_RXV4_STRUCT;

#define RXV5    CR_PHYMUX_22
typedef union _RO_RXV5_STRUCT {
	struct {
	UINT32 fagc_lna_gain:2;
	UINT32 rsv_0:5;
#define RXV5_MISC_OFDM_FOE              0x0fff
#define RXV5_MISC_OFDM_LTF_PROC_TIME    (0x7f << 12)
#define RXV5_MISC_OFDM_LTF_SNR          (0x3f << 19)
#define RXV5_MISC_CCK_FOE               0x07ff
#define RXV5_MISC_CCK_INIT_DFE_LMS_ERR  (0x3ff << 11)
	UINT32 misc:25;
	} field;
	UINT32 word;
} RO_RXV5_STRUCT;

#define RXV6    CR_PHYMUX_23
typedef union _RO_RXV6_STRUCT {
	struct {
	UINT32 nf0:8;
	UINT32 nf1:8;
	UINT32 nf2:8;
	UINT32 rx_valid_indicator:1;
	UINT32 nsts:3;
	UINT32 cags_state:3;
	UINT32 rsv_0:1;
	} field;
	UINT32 word;
} RO_RXV6_STRUCT;


#endif /* __WF_PHY_BACK_H__ */
