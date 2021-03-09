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
	wf_wtblon.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __WF_WTBL_ON_H__
#define __WF_WTBL_ON_H__

#define WF_WTBL_ON_BASE		0x28000

#define WTBL_WTBL1DW0			(WF_WTBL_ON_BASE + 0x00)
#define WTBL_ON_RIUCR0			(WF_WTBL_ON_BASE + 0x2200)	/* 0x2A200 */
#define WTBL_ON_RIUCR1			(WF_WTBL_ON_BASE + 0x2204)	/* 0x2A204 */
#define WTBL_ON_RIUCR2			(WF_WTBL_ON_BASE + 0x2208)	/* 0x2A208 */
#define WTBL_ON_RIUCR3			(WF_WTBL_ON_BASE + 0x220C)	/* 0x2A20C */

#ifdef RT_BIG_ENDIAN
union WTBL_1_DW0 {
	struct {
		UINT32 wm:1;
		UINT32 sw:1;
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
	UINT32 word;
};
#else
union WTBL_1_DW0 {
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
		UINT32 sw:1;
		UINT32 wm:1;
	} field;
	UINT32 word;
};
#endif


#define WTBL_WTBL1DW1		(WF_WTBL_ON_BASE + 0x04)	/* 0x28004 */
union WTBL_1_DW1 {
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
};


#define WTBL_WTBL1DW2		(WF_WTBL_ON_BASE + 0x08)	/* 0x28008 */
#ifdef RT_BIG_ENDIAN
union WTBL_1_DW2 {
	struct {
		UINT32 gid:1;
		UINT32 adm:1;
		UINT32 vht:1;
		UINT32 ht:1;
		UINT32 qos:1;
		UINT32 mesh:1;
		UINT32 txop_ps_cap:1;
		UINT32 tebf:1;
		UINT32 tibf:1;
		UINT32 ldpc:1;
		UINT32 dyn_bw:1;
		UINT32 baf_en:1;
		UINT32 smps:1;
		UINT32 rdg_ba:1;
		UINT32 cf_ack:1;
		UINT32 rts:1;
		UINT32 r:1;
		UINT32 rx_ps:1;
		UINT32 af:3;
		UINT32 dis_rhtr:1;
		UINT32 fd:1;
		UINT32 td:1;
		UINT32 wpi_even:1;
		UINT32 cipher_suit:4;
		UINT32 mm:3;
	} field;

	struct {
		UINT32 gid:1;
		UINT32 adm:1;
		UINT32 vht:1;
		UINT32 ht:1;
		UINT32 qos:1;
		UINT32 mesh:1;
		UINT32 txop_ps_cap:1;
		UINT32 tebf:1;
		UINT32 tibf:1;
		UINT32 ldpc:1;
		UINT32 dyn_bw:1;
		UINT32 baf_en:1;
		UINT32 smps:1;
		UINT32 rdg_ba:1;
		UINT32 cf_ack:1;
		UINT32 rts:1;
		UINT32 r:1;
		UINT32 tx_ps:1;
		UINT32 tebf_vht:1;
		UINT32 tibf_vht:1;
		UINT32 ldpc_vht:1;
		UINT32 dis_rhtr:1;
		UINT32 fd:1;
		UINT32 td:1;
		UINT32 wpi_even:1;
		UINT32 cipher_suit:4;
		UINT32 rsv1:3;
	} field_mt7636;
	UINT32 word;
};
#else
union WTBL_1_DW2 {
	struct {
		UINT32 mm:3;
		UINT32 cipher_suit:4;
		UINT32 wpi_even:1;
		UINT32 td:1;
		UINT32 fd:1;
		UINT32 dis_rhtr:1;
		UINT32 af:3;
		UINT32 rx_ps:1;
		UINT32 r:1;
		UINT32 rts:1;
		UINT32 cf_ack:1;
		UINT32 rdg_ba:1;
		UINT32 smps:1;
		UINT32 baf_en:1;
		UINT32 dyn_bw:1;
		UINT32 ldpc:1;
		UINT32 tibf:1;
		UINT32 tebf:1;
		UINT32 txop_ps_cap:1;
		UINT32 mesh:1;
		UINT32 qos:1;
		UINT32 ht:1;
		UINT32 vht:1;
		UINT32 adm:1;
		UINT32 gid:1;
	} field;

	struct {
		UINT32 rsv1:3;
		UINT32 cipher_suit:4;
		UINT32 wpi_even:1;
		UINT32 td:1;
		UINT32 fd:1;
		UINT32 dis_rhtr:1;
		UINT32 ldpc_vht:1;
		UINT32 tibf_vht:1;
		UINT32 tebf_vht:1;
		UINT32 tx_ps:1;
		UINT32 r:1;
		UINT32 rts:1;
		UINT32 cf_ack:1;
		UINT32 rdg_ba:1;
		UINT32 smps:1;
		UINT32 baf_en:1;
		UINT32 dyn_bw:1;
		UINT32 ldpc:1;
		UINT32 tibf:1;
		UINT32 tebf:1;
		UINT32 txop_ps_cap:1;
		UINT32 mesh:1;
		UINT32 qos:1;
		UINT32 ht:1;
		UINT32 vht:1;
		UINT32 adm:1;
		UINT32 gid:1;
	} field_mt7636;
	UINT32 word;
};
#endif

#define WTBL_WTBL1DW3		(WF_WTBL_ON_BASE + 0x0c)	/* 0x2800c */
#ifdef RT_BIG_ENDIAN
union WTBL_1_DW3 {
	struct {
		UINT32 skip_tx:1;
		UINT32 psm:1;
		UINT32 i_psm:1;
		UINT32 du_i_psm:1;
		UINT32 chk_per:1;
		UINT32 wtbl4_fid:11;
		UINT32 wtbl2_eid:5;
		UINT32 wtbl2_fid:11;
	} field;
	UINT32 word;
};
#else
union WTBL_1_DW3 {
	struct {
		UINT32 wtbl2_fid:11;
		UINT32 wtbl2_eid:5;
		UINT32 wtbl4_fid:11;
		UINT32 chk_per:1;
		UINT32 du_i_psm:1;
		UINT32 i_psm:1;
		UINT32 psm:1;
		UINT32 skip_tx:1;
	} field;
	UINT32 word;
};
#endif

#define WTBL_WTBL1DW4		(WF_WTBL_ON_BASE + 0x10)	/* 0x28010 */
#ifdef RT_BIG_ENDIAN
union WTBL_1_DW4 {
	struct {
		UINT32 partial_aid:9;
		UINT32 wtbl4_eid:6;
		UINT32 wtbl3_eid:6;
		UINT32 wtbl3_fid:11;
	} field;
	UINT32 word;
};
#else
union WTBL_1_DW4 {
	struct {
		UINT32 wtbl3_fid:11;
		UINT32 wtbl3_eid:6;
		UINT32 wtbl4_eid:6;
		UINT32 partial_aid:9;
	} field;
	UINT32 word;
};
#endif

#define WTBL_BTCRn			(WF_WTBL_ON_BASE + 0x2000)
#define WTBL_MBTCRn			(WF_WTBL_ON_BASE + 0x2010)
#define WTBL_BTBCRn			(WF_WTBL_ON_BASE + 0x2030)
#define WTBL_MBTBCRn		(WF_WTBL_ON_BASE + 0x2040)
#define WTBL_BRCRn			(WF_WTBL_ON_BASE + 0x2100)
#define WTBL_MBRCRn			(WF_WTBL_ON_BASE + 0x2110)
#define WTBL_BRBCRn			(WF_WTBL_ON_BASE + 0x2130)
#define WTBL_MBRBCRn		(WF_WTBL_ON_BASE + 0x2140)

#define WTBL1OR				(WF_WTBL_ON_BASE + 0x2300)
#define PSM_W_FLAG			(1 << 31)
#endif /* __WF_WTBL_ON_H__ */

