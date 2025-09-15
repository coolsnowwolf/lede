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


#define WTBL_BASE_ADDR			0x30000
#define WTBL_PER_ENTRY_SIZE		(0x100)

#define WTBL_DW0_MUARIDX_MASK	0x3f0000
#define WTBL_DW0_MUARIDX_BIT		16
#define WTBL_SET_MUARIDX(_x, _y)	(_x = (_x & (~(0x3f<<16))) | (((_y) & 0x3f)<<16))
#define WTBL_SET_RC_A1(_x, _y)		(_x = (_x & (~(0x1<<22))) | (((_y) & 0x1)<<22))
#define WTBL_SET_KID(_x, _y)		(_x = (_x & (~(0x3<<23))) | (((_y) & 0x3)<<23))
#define WTBL_SET_RCID(_x, _y)		(_x = (_x & (~(0x1<<25))) | (((_y) & 0x1)<<25))
#define WTBL_SET_RKV(_x, _y)		(_x = (_x & (~(0x1<<26))) | (((_y) & 0x1)<<26))
#define WTBL_SET_IKV(_x, _y)		(_x = (_x & (~(0x1<<27))) | (((_y) & 0x1)<<27))
#define WTBL_SET_RV(_x, _y)			(_x = (_x & (~(0x1<<28))) | (((_y) & 0x1)<<28))
#define WTBL_SET_RC_A2(_x, _y)		(_x = (_x & (~(0x1<<29))) | (((_y) & 0x1)<<29))

#define WTBL_SET_PEER_ADDR4(_x, _y)	(_x = (_x & (~(0xff))) | (((_y) & 0xff)<<0))
#define WTBL_SET_PEER_ADDR5(_x, _y)	(_x = (_x & (~(0xff<<8))) | (((_y) & 0xff)<<8))
#define WTBL_SET_PEER_ADDR0_3(_x, _y)	((_x) = ((_y) & 0xffffffff))

union WTBL_DW0 {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 gid_su:1;
		UINT32 wpi_flg:1;
		UINT32 rc_a2:1;
		UINT32 rv:1;
		UINT32 ikv:1;
		UINT32 rkv:1;
		UINT32 rc_id:1;
		UINT32 kid:2;
		UINT32 rc_a1:1;
		UINT32 muar_idx:6;
		UINT32 addr_5:8;
		UINT32 addr_4:8;
	} field;
#else
	struct {
		UINT32 addr_4:8;
		UINT32 addr_5:8;
		UINT32 muar_idx:6;
		UINT32 rc_a1:1;
		UINT32 kid:2;
		UINT32 rc_id:1;
		UINT32 rkv:1;
		UINT32 ikv:1;
		UINT32 rv:1;
		UINT32 rc_a2:1;
		UINT32 wpi_flg:1;
		UINT32 gid_su:1;
	} field;
#endif
	UINT32 word;
};


union WTBL_DW1 {
	struct {
		UINT32 addr_0;
	} field;
	UINT32 word;
};


enum WTBL_CIPHER_SUIT {
	WTBL_CIPHER_NONE = 0,
	WTBL_CIPHER_WEP_40 = 1,
	WTBL_CIPHER_TKIP_MIC = 2,
	WTBL_CIPHER_TKIP_NO_MIC = 3,
	WTBL_CIPHER_CCMP_128_PMF = 4,
	WTBL_CIPHER_WEP_104 = 5,
	WTBL_CIPHER_BIP_CMAC_128 = 6,
	WTBL_CIPHER_WEP_128 = 7,
	WTBL_CIPHER_WPI_128 = 8,
	WTBL_CIPHER_CCMP_128_CCX = 9,
	WTBL_CIPHER_CCMP_256 = 10,
	WTBL_CIPHER_GCMP_128 = 11,
	WTBL_CIPHER_GCMP_256 = 12,
	WTBL_CIPHER_GCMP_WPI_128 = 13,
};


#define WTBL_SET_DIS_RHTR(_x, _y)		(_x = (_x & (~(0x1<<1))) | (((_y) & 0x1)<<1))
#define WTBL_SET_WPI_EVEN(_x, _y)		(_x = (_x & (~(0x1<<2))) | (((_y) & 0x1)<<2))
#define WTBL_SET_AAD_OM(_x, _y)		(_x = (_x & (~(0x1<<3))) | (((_y) & 0x1)<<3))
#define WTBL_SET_CIPHER_SUIT(_x, _y)	(_x = (_x & (~(0xf<<4))) | (((_y) & 0xf)<<4))
#define WTBL_SET_PFMU_IDX(_x, _y)		(_x = (_x & (~(0xff<<8))) | (((_y) & 0xff)<<8))
#define WTBL_SET_PARTIAL_AID(_x, _y)	(_x = (_x & (~(0x1ff<<16))) | (((_y) & 0x1ff)<<16))
#define WTBL_SET_TIBF_HT(_x, _y)		(_x = (_x & (~(0x1<<25))) | (((_y) & 0x1)<<25))
#define WTBL_SET_TEBF_HT(_x, _y)		(_x = (_x & (~(0x1<<26))) | (((_y) & 0x1)<<26))
#define WTBL_SET_TIBF_VHT(_x, _y)		(_x = (_x & (~(0x1<<27))) | (((_y) & 0x1)<<27))
#define WTBL_SET_TEBF_VHT(_x, _y)		(_x = (_x & (~(0x1<<28))) | (((_y) & 0x1)<<28))
#define WTBL_SET_HT(_x, _y)				(_x = (_x & (~(0x1<<29))) | (((_y) & 0x1)<<29))
#define WTBL_SET_VHT(_x, _y)			(_x = (_x & (~(0x1<<30))) | (((_y) & 0x1)<<30))
#define WTBL_SET_MESH(_x, _y)			(_x = (_x & (~(0x1<<31))) | (((_y) & 0x1)<<31))

union WTBL_DW2 {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 mesh:1;
		UINT32 vht:1;
		UINT32 ht:1;
		UINT32 tebf_vht:1;
		UINT32 tibf_vht:1;
		UINT32 tebf:1;
		UINT32 tibf:1;
		UINT32 partial_aid:9;
		UINT32 pfmu_idx:8;
		UINT32 cipher_suit:4;
		UINT32 AAD_OM:1;
		UINT32 wpi_even:1;
		UINT32 dis_rhtr:1;
		UINT32 SW:1;
	} field;
#else
	struct {
		UINT32 SW:1;
		UINT32 dis_rhtr:1;
		UINT32 wpi_even:1;
		UINT32 AAD_OM:1;
		UINT32 cipher_suit:4;
		UINT32 pfmu_idx:8;
		UINT32 partial_aid:9;
		UINT32 tibf:1;
		UINT32 tebf:1;
		UINT32 tibf_vht:1;
		UINT32 tebf_vht:1;
		UINT32 ht:1;
		UINT32 vht:1;
		UINT32 mesh:1;
	} field;
#endif
	UINT32 word;
};

#define DMAC_WTBL_DW3_SET_BAF(_x, _y)		(_x = ((_x) & (~0x00001000)) | (((_y) & 0x1)<<12))
#define DMAC_WTBL_DW3_SET_RDGBA(_x, _y)	(_x = ((_x) & (~0x00004000)) | (((_y) & 0x1)<<14))
#define DMAC_WTBL_DW3_SET_R(_x, _y)		(_x = ((_x) & (~0x00008000)) | (((_y) & 0x1)<<15))
#define DMAC_WTBL_DW3_SET_SMPS(_x, _y)	(_x = ((_x) & (~0x00800000)) | (((_y) & 0x1)<<23))
#define DMAC_WTBL_DW3_SET_RTS(_x, _y)		(_x = ((_x) & (~0x00400000)) | (((_y) & 0x1)<<22))
#define DMAC_WTBL_DW3_SET_TXPS(_x, _y)	(_x = ((_x) & (~0x00200000)) | (((_y) & 0x1)<<21))
#define DMAC_WTBL_DW3_SET_AF(_x, _y)		(_x = ((_x) & (~0x07000000)) | (((_y) & 0x7)<<24))
#define DMAC_WTBL_DW3_SET_DU_IPSM(_x, _y)	(_x = ((_x) & (~0x10000000)) | (((_y) & 0x1)<<28))
#define DMAC_WTBL_DW3_SET_IPSM(_x, _y)		(_x = ((_x) & (~0x20000000)) | (((_y) & 0x1)<<29))
#ifdef RT_BIG_ENDIAN
union WTBL_DW3 {
	struct {
		UINT32 skip_tx:1;
		UINT32 psm:1;
		UINT32 i_psm:1;
		UINT32 du_i_psm:1;
		UINT32 txop_ps_cap:1;
		UINT32 af:3;
		UINT32 smps:1;
		UINT32 rts:1;
		UINT32 tx_ps:1;
		UINT32 spe_idx:5;
		UINT32 r:1;
		UINT32 rdg_ba:1;
		UINT32 cf_ack:1;
		UINT32 baf_en:1;
		UINT32 ant_id_sts_3:3;
		UINT32 ant_id_sts_2:3;
		UINT32 ant_id_sts_1:3;
		UINT32 ant_id_sts_0:3;
	} field;
	UINT32 word;
};
#else
union WTBL_DW3 {
	struct {
		UINT32 ant_id_sts_0:3;
		UINT32 ant_id_sts_1:3;
		UINT32 ant_id_sts_2:3;
		UINT32 ant_id_sts_3:3;
		UINT32 baf_en:1;
		UINT32 cf_ack:1;
		UINT32 rdg_ba:1;
		UINT32 r:1;
		UINT32 spe_idx:5;
		UINT32 tx_ps:1;
		UINT32 rts:1;
		UINT32 smps:1;
		UINT32 af:3;
		UINT32 txop_ps_cap:1;
		UINT32 du_i_psm:1;
		UINT32 i_psm:1;
		UINT32 psm:1;
		UINT32 skip_tx:1;
	} field;
	UINT32 word;
};
#endif


union WTBL_DW4 {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 ba_en:8;
		UINT32 ba_win_size_tid_7:3;
		UINT32 ba_win_size_tid_6:3;
		UINT32 ba_win_size_tid_5:3;
		UINT32 ba_win_size_tid_4:3;
		UINT32 ba_win_size_tid_3:3;
		UINT32 ba_win_size_tid_2:3;
		UINT32 ba_win_size_tid_1:3;
		UINT32 ba_win_size_tid_0:3;
	} field;
#else
	struct {
		UINT32 ba_win_size_tid_0:3;
		UINT32 ba_win_size_tid_1:3;
		UINT32 ba_win_size_tid_2:3;
		UINT32 ba_win_size_tid_3:3;
		UINT32 ba_win_size_tid_4:3;
		UINT32 ba_win_size_tid_5:3;
		UINT32 ba_win_size_tid_6:3;
		UINT32 ba_win_size_tid_7:3;
		UINT32 ba_en:8;
	} field;
#endif
	UINT32 word;
};


#define WTBL_SET_TXPWR_OFFSET(_x, _y)	(_x = ((_x) & (~0x0000001f)) | (((_y) & 0x1f)<<0))
#define WTBL_SET_FCAP(_x, _y)			(_x = ((_x) & (~0x00003000)) | (((_y) & 0x3)<<12))
#define WTBL_SET_LDPC(_x, _y)			(_x = ((_x) & (~0x00004000)) | (((_y) & 0x1)<<14))
#define WTBL_SET_LDPC_VHT(_x, _y)		(_x = ((_x) & (~0x00008000)) | (((_y) & 0x1)<<12))
#define WTBL_SET_MM(_x, _y)				(_x = ((_x) & (~0x00070000)) | (((_y) & 0x7)<<16))
#define WTBL_SET_QOS(_x, _y)			(_x = ((_x) & (~0x00080000)) | (((_y) & 0x1)<<19))
#define WTBL_SET_FD(_x, _y)				(_x = ((_x) & (~0x00100000)) | (((_y) & 0x1)<<20))
#define WTBL_SET_TD(_x, _y)				(_x = ((_x) & (~0x00200000)) | (((_y) & 0x1)<<21))
#define WTBL_SET_DYN_BW(_x, _y)		(_x = ((_x) & (~0x00400000)) | (((_y) & 0x1)<<22))

union WTBL_DW5 {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 rate_idx:3;
		UINT32 mpdu_ok_cnt:3;
		UINT32 mpdu_fail_cnt:3;
		UINT32 dyn_bw:1;
		UINT32 td:1;
		UINT32 fd:1;
		UINT32 qos:1;
		UINT32 mm:3;
		UINT32 ldpc_vht:1;
		UINT32 ldpc:1;
		UINT32 fcap:2;
		UINT32 g16:1;
		UINT32 g8:1;
		UINT32 g4:1;
		UINT32 g2:1;
		UINT32 cbrn:3;
		UINT32 txpwr_offset:5;
	} field;
#else
	struct {
		UINT32 txpwr_offset:5;
		UINT32 cbrn:3;
		UINT32 g2:1;
		UINT32 g4:1;
		UINT32 g8:1;
		UINT32 g16:1;
		UINT32 fcap:2;
		UINT32 ldpc:1;
		UINT32 ldpc_vht:1;
		UINT32 mm:3;
		UINT32 qos:1;
		UINT32 fd:1;
		UINT32 td:1;
		UINT32 dyn_bw:1;
		UINT32 mpdu_fail_cnt:3;
		UINT32 mpdu_ok_cnt:3;
		UINT32 rate_idx:3;
	} field;
#endif
	UINT32 word;
};


#define WTBL_SET_RATE1(_x, _y)				(_x = ((_x) & (~0x00000fff)) | (((_y) & 0xfff)<<0))
#define WTBL_SET_RATE2(_x, _y)				(_x = ((_x) & (~0x00fff000)) | (((_y) & 0xfff)<<12))
#define WTBL_SET_RATE3_LOW(_x, _y)			(_x = ((_x) & (~0xff000000)) | (((_y) & 0xfff)<<24))
#define WTBL_SET_RATE3_HIGH(_x, _y)		(_x = ((_x) & (~0x0000000f)) | ((((_y) & 0xfff) >> 8) << 0))
#define WTBL_SET_RATE4(_x, _y)				(_x = ((_x) & (~0x0000fff0)) | (((_y) & 0xfff) << 4))
#define WTBL_SET_RATE5(_x, _y)				(_x = ((_x) & (~0x0fff0000)) | (((_y) & 0xfff)<<16))
#define WTBL_SET_RATE6_LOW(_x, _y)			(_x = ((_x) & (~0xf0000000)) | (((_y) & 0xfff)<<28))
#define WTBL_SET_RATE6_HIGH(_x, _y)		(_x = ((_x) & (~0x000000ff)) | ((((_y) & 0xfff) >> 4) << 0))
#define WTBL_SET_RATE7(_x, _y)				(_x = ((_x) & (~0x000fff00)) | (((_y) & 0xfff) << 8))
#define WTBL_SET_RATE8(_x, _y)				(_x = ((_x) & (~0xfff00000)) | (((_y) & 0xfff)<<20))

union WTBL_DW6 {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 rate_3_low:8;
		UINT32 rate_2:12;
		UINT32 rate_1:12;
	} field;
#else
	struct {
		UINT32 rate_1:12;
		UINT32 rate_2:12;
		UINT32 rate_3_low:8;
	} field;
#endif
	UINT32 word;
};


union WTBL_DW7 {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 rate_6_low:4;
		UINT32 rate_5:12;
		UINT32 rate_4:12;
		UINT32 rate_3_high:4;
	} field;
#else
	struct {
		UINT32 rate_3_high:4;
		UINT32 rate_4:12;
		UINT32 rate_5:12;
		UINT32 rate_6_low:4;
	} field;
#endif
	UINT32 word;
};


union WTBL_DW8 {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 rate_8:12;
		UINT32 rate_7:12;
		UINT32 rate_6_high:8;
	} field;
#else
	struct {
		UINT32 rate_6_high:8;
		UINT32 rate_7:12;
		UINT32 rate_8:12;
	} field;
#endif
	UINT32 word;
};


union WTBL_DW9 {
	UINT32 word;
};


union WTBL_DW10 {
	UINT32 word;
};


union WTBL_DW11 {
	UINT32 word;
};

union WTBL_DW12 {
	UINT32 word;
};

union WTBL_DW13 {
	UINT32 word;
};

union WTBL_DW14 {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 rate_1_fail_cnt:16;
		UINT32 rate_1_tx_cnt:16;
	} field;
#else
	struct {
		UINT32 rate_1_tx_cnt:16;
		UINT32 rate_1_fail_cnt:16;
	} field;
#endif
	UINT32 word;
};


union WTBL_DW15 {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 rate_3_ok_cnt:16;
		UINT32 rate_2_ok_cnt:16;
	} field;
#else
	struct {
		UINT32 rate_2_ok_cnt:16;
		UINT32 rate_3_ok_cnt:16;
	} field;
#endif
	UINT32 word;
};


union WTBL_DW16 {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 current_bw_fail_cnt:16;
		UINT32 current_bw_tx_cnt:16;
	} field;
#else
	struct {
		UINT32 current_bw_tx_cnt:16;
		UINT32 current_bw_fail_cnt:16;
	} field;
#endif
	UINT32 word;
};


union WTBL_DW17 {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 other_bw_fail_cnt:16;
		UINT32 other_bw_tx_cnt:16;
	} field;
#else
	struct {
		UINT32 other_bw_tx_cnt:16;
		UINT32 other_bw_fail_cnt:16;
	} field;
#endif
	UINT32 word;
};

union WTBL_DW18 {
	UINT32 word;
};

union WTBL_DW19 {
	UINT32 word;
};

union WTBL_DW20 {
	UINT32 word;
};

union WTBL_DW21 {
	UINT32 word;
};

union WTBL_DW22 {
	UINT32 word;
};

union WTBL_DW23 {
	UINT32 word;
};

union WTBL_DW24 {
	UINT32 word;
};

union WTBL_DW25 {
	UINT32 word;
};

union WTBL_DW26 {
	UINT32 word;
};

union WTBL_DW27 {
	UINT32 word;
};

union WTBL_DW28 {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32 resp_rcpi_3:8;
		UINT32 resp_rcpi_2:8;
		UINT32 resp_rcpi_1:8;
		UINT32 resp_rcpi_0:8;
	} field;
#else
	struct {
		UINT32 resp_rcpi_0:8;
		UINT32 resp_rcpi_1:8;
		UINT32 resp_rcpi_2:8;
		UINT32 resp_rcpi_3:8;
	} field;
#endif
	UINT32 word;
};

union WTBL_DW29 {
	UINT32 word;
};


struct wtbl_key_tb {
	UINT32 key[16];	/* wtbl_d30 ~ wtbl_d45 */
};


struct wtbl_rx_stat {
	union WTBL_DW27 wtbl_d27;
	union WTBL_DW28 wtbl_d28;
	union WTBL_DW29 wtbl_d29;
};


struct wtbl_adm_ctrl {
	union WTBL_DW19 wtbl_d19;
	union WTBL_DW20 wtbl_d20;
	union WTBL_DW21 wtbl_d21;
	union WTBL_DW22 wtbl_d22;
	union WTBL_DW23 wtbl_d23;
	union WTBL_DW24 wtbl_d24;
	union WTBL_DW25 wtbl_d25;
	union WTBL_DW26 wtbl_d26;
};


struct wtbl_retry_cnt {
	union WTBL_DW18 wtbl_d18;
};

struct wtbl_rate_counter {
	union WTBL_DW14 wtbl_d14;
	union WTBL_DW15 wtbl_d15;
	union WTBL_DW16 wtbl_d16;
	union WTBL_DW17 wtbl_d17;
};


struct wtbl_serial_num {
	union WTBL_DW9 wtbl_d9;
	union WTBL_DW10 wtbl_d10;
	union WTBL_DW11 wtbl_d11;
	union WTBL_DW12 wtbl_d12;
	union WTBL_DW13 wtbl_d13;
};


struct wtbl_rate_tb {
	union WTBL_DW6 wtbl_d6;
	union WTBL_DW7 wtbl_d7;
	union WTBL_DW8 wtbl_d8;
};

struct wtbl_tx_rx_cap {
	union WTBL_DW2 wtbl_d2;
	union WTBL_DW3 wtbl_d3;
	union WTBL_DW4 wtbl_d4;
	union WTBL_DW5 wtbl_d5;
};

struct wtbl_basic_info {
	union WTBL_DW0 wtbl_d0;
	union WTBL_DW1 wtbl_d1;
};

/*
	WTBL segment 1 definitions
*/
typedef struct GNU_PACKED wtbl_struc {
	struct wtbl_basic_info peer_basic_info;
	struct wtbl_tx_rx_cap trx_cap;
	struct wtbl_rate_tb auto_rate_tb;
	struct wtbl_serial_num serial_no;
	struct wtbl_rate_counter rate_counters;
	struct wtbl_retry_cnt retry_cnt;
	struct wtbl_adm_ctrl adm_ctrl;
	struct wtbl_rx_stat rx_stat;
	struct wtbl_key_tb key_tb;
} WTBL_STRUC;

#endif /* __WF_WTBL_H__ */

