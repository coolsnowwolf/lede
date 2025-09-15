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
	wf_wtbl.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_WTBL_H__
#define __WF_WTBL_H__

/*
	WTBL segment 1 definitions
*/
typedef struct GNU_PACKED wtbl_1_struc {
	union WTBL_1_DW0 wtbl_1_d0;
	union WTBL_1_DW1 wtbl_1_d1;
	union WTBL_1_DW2 wtbl_1_d2;
	union WTBL_1_DW3 wtbl_1_d3;
	union WTBL_1_DW4 wtbl_1_d4;
	UINT32 dummy[3];
} WTBL_1_STRUC;


/*
	WTBL segment 2 definitions
*/
union WTBL_2_DW0 {
	UINT32 pn_0;
	UINT32 word;
};

#ifdef RT_BIG_ENDIAN
union WTBL_2_DW1 {
	struct {
		UINT32 rsv_1_28:4;
		UINT32 com_sn:12;
		UINT32 pn_32:16;
	} field;
	UINT32 word;
};
#else
union WTBL_2_DW1 {
	struct {
		UINT32 pn_32:16;
		UINT32 com_sn:12;
		UINT32 rsv_1_28:4;
	} field;
	UINT32 word;
};
#endif

#ifdef RT_BIG_ENDIAN
union WTBL_2_DW2 {
	struct {
		UINT32 tid_ac_2_sn_0:8;
		UINT32 tid_ac_1_sn:12;
		UINT32 tid_ac_0_sn:12;
	} field;
	UINT32 word;
};
#else
union WTBL_2_DW2 {
	struct {
		UINT32 tid_ac_0_sn:12;
		UINT32 tid_ac_1_sn:12;
		UINT32 tid_ac_2_sn_0:8;
	} field;
	UINT32 word;
};
#endif

#ifdef RT_BIG_ENDIAN
union WTBL_2_DW3 {
	struct {
		UINT32 tid_5_sn_0:4;
		UINT32 tid_4_sn:12;
		UINT32 tid_ac_3_sn:12;
		UINT32 tid_ac_2_sn_9:4;
	} field;
	UINT32 word;
};
#else
union WTBL_2_DW3 {
	struct {
		UINT32 tid_ac_2_sn_9:4;
		UINT32 tid_ac_3_sn:12;
		UINT32 tid_4_sn:12;
		UINT32 tid_5_sn_0:4;
	} field;
	UINT32 word;
};
#endif

#ifdef RT_BIG_ENDIAN
union WTBL_2_DW4 {
	struct {
		UINT32 tid_7_sn:12;
		UINT32 tid_6_sn:12;
		UINT32 tid_5_sn_5:8;
	} field;
	UINT32 word;
};
#else
union WTBL_2_DW4 {
	struct {
		UINT32 tid_5_sn_5:8;
		UINT32 tid_6_sn:12;
		UINT32 tid_7_sn:12;
	} field;
	UINT32 word;
};
#endif

#ifdef RT_BIG_ENDIAN
union WTBL_2_DW5 {
	struct {
		UINT32 rate_1_fail_cnt:16;
		UINT32 rate_1_tx_cnt:16;
	} field;
	UINT32 word;
};
#else
union WTBL_2_DW5 {
	struct {
		UINT32 rate_1_tx_cnt:16;
		UINT32 rate_1_fail_cnt:16;
	} field;
	UINT32 word;
};
#endif

#ifdef RT_BIG_ENDIAN
union WTBL_2_DW6 {
	struct {
		UINT32 rate_5_tx_cnt:8;
		UINT32 rate_4_tx_cnt:8;
		UINT32 rate_3_tx_cnt:8;
		UINT32 rate_2_tx_cnt:8;
	} field;
	UINT32 word;
};
#else
union WTBL_2_DW6 {
	struct {
		UINT32 rate_2_tx_cnt:8;
		UINT32 rate_3_tx_cnt:8;
		UINT32 rate_4_tx_cnt:8;
		UINT32 rate_5_tx_cnt:8;
	} field;
	UINT32 word;
};
#endif

#ifdef RT_BIG_ENDIAN
union WTBL_2_DW7 {
	struct {
		UINT32 current_bw_fail_cnt:16;
		UINT32 current_bw_tx_cnt:16;
	} field;
	UINT32 word;
};
#else
union WTBL_2_DW7 {
	struct {
		UINT32 current_bw_tx_cnt:16;
		UINT32 current_bw_fail_cnt:16;
	} field;
	UINT32 word;
};
#endif

#ifdef RT_BIG_ENDIAN
union WTBL_2_DW8 {
	struct {
		UINT32 other_bw_fail_cnt:16;
		UINT32 other_bw_tx_cnt:16;
	} field;
	UINT32 word;
};
#else
union WTBL_2_DW8 {
	struct {
		UINT32 other_bw_tx_cnt:16;
		UINT32 other_bw_fail_cnt:16;
	} field;
	UINT32 word;
};
#endif

#ifdef RT_BIG_ENDIAN
union WTBL_2_DW9 {
	struct {
		UINT32 rate_idx:3;
		UINT32 mpdu_ok_cnt:3;
		UINT32 mpdu_fail_cnt:3;
		UINT32 rsv_9_20:3;
		UINT32 g16:1;
		UINT32 g8:1;
		UINT32 g4:1;
		UINT32 g2:1;
		UINT32 fcap:2;
		UINT32 cbrn:3;
		UINT32 ccbw_sel:2;
		UINT32 ant_pri:3;
		UINT32 spe_en:1;
		UINT32 tx_pwr_offset:5;
	} field;
	UINT32 word;
};
#else
union WTBL_2_DW9 {
	struct {
		UINT32 tx_pwr_offset:5;
		UINT32 spe_en:1;
		UINT32 ant_pri:3;
		UINT32 ccbw_sel:2;
		UINT32 cbrn:3;
		UINT32 fcap:2;
		UINT32 g2:1;
		UINT32 g4:1;
		UINT32 g8:1;
		UINT32 g16:1;
		UINT32 rsv_9_20:3;
		UINT32 mpdu_fail_cnt:3;
		UINT32 mpdu_ok_cnt:3;
		UINT32 rate_idx:3;
	} field;
	UINT32 word;
};
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED wtbl_tx_rate {
	UINT16 rsv_12:4;
	UINT16 stbc:1;
	UINT16 n_sts:2;
	UINT16 tx_mode:3;
	UINT16 tx_rate:6;
} WTBL_TX_RATE;
#else
typedef struct GNU_PACKED wtbl_tx_rate {
	UINT16 tx_rate:6;
	UINT16 tx_mode:3;
	UINT16 n_sts:2;
	UINT16 stbc:1;
	UINT16 rsv_12:4;
} WTBL_TX_RATE;
#endif


#ifdef RT_BIG_ENDIAN
union WTBL_2_DW10 {
	struct {
		UINT32 rate_3_0:8;
		UINT32 rate_2:12;
		UINT32 rate_1:12;  /* format refer to WTBL_TX_RATE */
	} field;
	UINT32 word;
};
#else
union WTBL_2_DW10 {
	struct {
		UINT32 rate_1:12;  /* format refer to WTBL_TX_RATE */
		UINT32 rate_2:12;
		UINT32 rate_3_0:8;
	} field;
	UINT32 word;
};
#endif

#ifdef RT_BIG_ENDIAN
union WTBL_2_DW11 {
	struct {
		UINT32 rate_6_0:4;
		UINT32 rate_5:12;
		UINT32 rate_4:12;
		UINT32 rate_3_8:4;
	} field;
	UINT32 word;
};
#else
union WTBL_2_DW11 {
	struct {
		UINT32 rate_3_8:4;
		UINT32 rate_4:12;
		UINT32 rate_5:12;
		UINT32 rate_6_0:4;
	} field;
	UINT32 word;
};
#endif

#ifdef RT_BIG_ENDIAN
union WTBL_2_DW12 {
	struct {
		UINT32 rate_8:12;
		UINT32 rate_7:12;
		UINT32 rate_6_4:8;
	} field;
	UINT32 word;
};
#else
union WTBL_2_DW12 {
	struct {
		UINT32 rate_6_4:8;
		UINT32 rate_7:12;
		UINT32 rate_8:12;
	} field;
	UINT32 word;
};
#endif

#ifdef RT_BIG_ENDIAN
union WTBL_2_DW13 {
	struct {
		UINT32 rsv_13_24:8;
		UINT32 resp_rcpi_2:8;
		UINT32 resp_rcpi_1:8;
		UINT32 resp_rcpi_0:8;
	} field;

	struct {
		UINT32 af:3;
		UINT32 mm:3;
		UINT32 rsv_13_24:2;
		UINT32 resp_rcpi_2:8;
		UINT32 resp_rcpi_1:8;
		UINT32 resp_rcpi_0:8;
	} field_mt7636;

	UINT32 word;
};
#else
union WTBL_2_DW13 {
	struct {
		UINT32 resp_rcpi_0:8;
		UINT32 resp_rcpi_1:8;
		UINT32 resp_rcpi_2:8;
		UINT32 rsv_13_24:8;
	} field;

	struct {
		UINT32 resp_rcpi_0:8;
		UINT32 resp_rcpi_1:8;
		UINT32 resp_rcpi_2:8;
		UINT32 rsv_13_24:2;
		UINT32 mm:3;
		UINT32 af:3;
	} field_mt7636;

	UINT32 word;
};
#endif

#ifdef RT_BIG_ENDIAN
union WTBL_2_DW14 {
	struct {
		UINT32 ant_sel:6;
		UINT32 cc_noise_sel:1;
		UINT32 ce_rmsd:4;
		UINT32 sts_3_ch_cap_noise:7;
		UINT32 sts_2_ch_cap_noise:7;
		UINT32 sts_1_ch_cap_noise:7;
	} field;
	UINT32 word;
};
#else
union WTBL_2_DW14 {
	struct {
		UINT32 sts_1_ch_cap_noise:7;
		UINT32 sts_2_ch_cap_noise:7;
		UINT32 sts_3_ch_cap_noise:7;
		UINT32 ce_rmsd:4;
		UINT32 cc_noise_sel:1;
		UINT32 ant_sel:6;
	} field;
	UINT32 word;
};
#endif

#ifdef RT_BIG_ENDIAN
union WTBL_2_DW15 {
	struct {
		UINT32 ba_en:8;
		UINT32 ba_win_size_tid:24;
	} field;
	UINT32 word;
};
#else
union WTBL_2_DW15 {
	struct {
		UINT32 ba_win_size_tid:24;
		UINT32 ba_en:8;
	} field;
	UINT32 word;
};
#endif

typedef struct GNU_PACKED wtbl_2_struc {
	union WTBL_2_DW0 wtbl_2_d0;
	union WTBL_2_DW1 wtbl_2_d1;
	union WTBL_2_DW2 wtbl_2_d2;
	union WTBL_2_DW3 wtbl_2_d3;
	union WTBL_2_DW4 wtbl_2_d4;
	union WTBL_2_DW5 wtbl_2_d5;
	union WTBL_2_DW6 wtbl_2_d6;
	union WTBL_2_DW7 wtbl_2_d7;
	union WTBL_2_DW8 wtbl_2_d8;
	union WTBL_2_DW9 wtbl_2_d9;
	union WTBL_2_DW10 wtbl_2_d10;
	union WTBL_2_DW11 wtbl_2_d11;
	union WTBL_2_DW12 wtbl_2_d12;
	union WTBL_2_DW13 wtbl_2_d13;
	union WTBL_2_DW14 wtbl_2_d14;
	union WTBL_2_DW15 wtbl_2_d15;
} WTBL_2_STRUC;


/*
	WTBL segment 3 definitions
*/

/* Used for Cipher Suit != (WEP_xx | WPI) */
struct GNU_PACKED wtbl_cipher_128 {
	UINT32 cipher_key_0;
	UINT32 cipher_key_32;
	UINT32 cipher_key_64;
	UINT32 cipher_key_96;

	UINT32 rx_mic_bip_key_0;
	UINT32 rx_mic_bip_key_32;

	UINT32 tx_mic_bip_key_0;
	UINT32 tx_mic_bip_key_32;
};


/* Used for CCMP-256 /GCMP-256, store cipher or MIC key only */
struct GNU_PACKED wtbl_cipher_256 {
	UINT32 key_0;
	UINT32 key_1;
	UINT32 key_2;
	UINT32 key_3;
	UINT32 key_4;
	UINT32 key_5;
	UINT32 key_6;
	UINT32 key_7;
};


/* Used for Cipher Suit == WPI */
struct GNU_PACKED wtbl_cipher_wpi {
	UINT32 cipher_key0_0;
	UINT32 cipher_key0_1;
	UINT32 cipher_key0_2;
	UINT32 cipher_key0_3;

	UINT32 mic_key0_0;
	UINT32 mic_key0_1;
	UINT32 mic_key0_2;
	UINT32 mic_key0_3;

	UINT32 cipher_key1_0;
	UINT32 cipher_key1_1;
	UINT32 cipher_key1_2;
	UINT32 cipher_key1_3;

	UINT32 mic_key1_0;
	UINT32 mic_key1_1;
	UINT32 mic_key1_2;
	UINT32 mic_key1_3;
};


/* Used for Cipher Suit == WEP */
struct GNU_PACKED wtbl_cipher_wep {
	UINT32 cipher_key0_0;
	UINT32 cipher_key0_1;
	UINT32 cipher_key0_2;
	UINT32 cipher_key0_3;

	UINT32 cipher_key1_0;
	UINT32 cipher_key1_1;
	UINT32 cipher_key1_2;
	UINT32 cipher_key1_3;

	UINT32 cipher_key2_0;
	UINT32 cipher_key2_1;
	UINT32 cipher_key2_2;
	UINT32 cipher_key2_3;

	UINT32 cipher_key3_0;
	UINT32 cipher_key3_1;
	UINT32 cipher_key3_2;
	UINT32 cipher_key3_3;
};


typedef union GNU_PACKED wtbl_3_struc {
	struct wtbl_cipher_wep cipher_wep;
	struct wtbl_cipher_wpi cipher_wpi;
	struct wtbl_cipher_128 cipher_128;
	struct wtbl_cipher_256 cipher_256;
} WTBL_3_STRUC;


/*
	WTBL segment 4 definitions
*/
#ifdef RT_BIG_ENDIAN
union wtbl_4_ac {
	struct {
		/* DWORD 0 */
		UINT32 rsv_0_24:8;
		UINT32 ac_ctt:24;

		/* DWORD 1 */
		UINT32 ac_ctb;
	} field;
	UINT32 word[2];
};
#else
union wtbl_4_ac {
	struct {
		/* DWORD 0 */
		UINT32 ac_ctt:24;
		UINT32 rsv_0_24:8;

		/* DWORD 1 */
		UINT32 ac_ctb;
	} field;
	UINT32 word[2];
};
#endif

typedef struct GNU_PACKED wtbl_4_struc {
	union wtbl_4_ac ac0;
	union wtbl_4_ac ac1;
	union wtbl_4_ac ac2;
	union wtbl_4_ac ac3;
} WTBL_4_STRUC;

#endif /* __WF_WTBL_H__ */

