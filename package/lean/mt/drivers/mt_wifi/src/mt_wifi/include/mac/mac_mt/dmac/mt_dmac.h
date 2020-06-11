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
	mt_dmac.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __MT_DMAC_H__
#define __MT_DMAC_H__

#include "mac/mac_mt/top.h"
#include "mac/mac_mt/dmac/gpio.h"
#include "mac/mac_mt/dmac/wf_agg.h"
#include "mac/mac_mt/dmac/wf_aon.h"
#include "mac/mac_mt/dmac/wf_arb.h"
#include "mac/mac_mt/dmac/wf_cfg.h"
#include "mac/mac_mt/dmac/wf_cfgoff.h"
#include "mac/mac_mt/dmac/wf_dma.h"
#include "mac/mac_mt/dmac/wf_lpon_top.h"
#include "mac/mac_mt/dmac/wf_mib.h"
#include "mac/mac_mt/dmac/wf_pf.h"
#include "mac/mac_mt/dmac/wf_rmac.h"
#include "mac/mac_mt/dmac/wf_sec.h"
#include "mac/mac_mt/dmac/wf_tmac.h"
#include "mac/mac_mt/dmac/wf_trb.h"
#include "mac/mac_mt/dmac/wf_wtbloff.h"
#include "mac/mac_mt/dmac/wf_wtblon.h"
#include "mac/mac_mt/dmac/wf_int_wakeup_top.h"
#include "mac/mac_mt/dmac/client.h"
#include "mac/mac_mt/dmac/wf_wtbl.h"
#include "mac/mac_mt/dmac/wf_phy.h"
#include "mac/mac_mt/dmac/wf_ple.h"
#include "mac/mac_mt/dmac/pse.h"
#include "mac/mac_mt/dmac/wf_pp.h"
#include "mac/mac_mt/dmac/wf_etbf.h"
#include "mac/mac_mt/dmac/hw_amsdu.h"
#ifdef CFG_SUPPORT_MU_MIMO
#ifdef MANUAL_MU
#include "mac/mac_mt/dmac/wf_mu.h"
#endif /* MANUAL_MU */
#endif /* CFG_SUPPORT_MU_MIMO */

#ifdef MT7622
#include "mac/mac_mt/dmac/dma_sch.h"
#endif /* MT7622 */




/*
	TX / RX descriptor format

	TX:
		PCI/RBUS_DMA_Descriptor + TXD + 802.11

	Rx:
		PCI/RBUS/USB_DMA_Descripotr + RXD + 802.11 + RX_CSO_INFO
*/


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TMAC_TXD_0 {
	/* DWORD 0 */
	UINT32 p_idx : 1;
	UINT32 q_idx : 5;
	UINT32 rsv_25 : 1;
	UINT32 UdpTcpChkSumOffload : 1;
	UINT32 IpChkSumOffload : 1;
	UINT32 EthTypeOffset : 7;
	UINT32 TxByteCount : 16;
} TMAC_TXD_0;
#else
typedef struct GNU_PACKED _TMAC_TXD_0 {
	/* DWORD 0 */
	UINT32 TxByteCount : 16;
	UINT32 EthTypeOffset : 7;
	UINT32 IpChkSumOffload : 1;
	UINT32 UdpTcpChkSumOffload : 1;
	UINT32 rsv_25 : 1;
	UINT32 q_idx : 5;
	UINT32 p_idx : 1;
} TMAC_TXD_0;
#endif

/* TMAC_TXD_0.PIdx */
#define P_IDX_LMAC	0
#define P_IDX_MCU	1
#define P_IDX_PLE_CTRL_PSE_PORT_3   3

#define UMAC_PLE_CTRL_P3_Q_0X1E     0x1e
#define UMAC_PLE_CTRL_P3_Q_0X1F     0x1f

/* TMAC_TXD_0.QIdx */
#define TxQ_IDX_AC0	0x00
#define TxQ_IDX_AC1	0x01
#define TxQ_IDX_AC2	0x02
#define TxQ_IDX_AC3	0x03
#define TxQ_IDX_AC10	0x04
#define TxQ_IDX_AC11	0x05
#define TxQ_IDX_AC12	0x06
#define TxQ_IDX_AC13	0x07
#define TxQ_IDX_AC20	0x08
#define TxQ_IDX_AC21	0x09
#define TxQ_IDX_AC22	0x0a
#define TxQ_IDX_AC23	0x0b
#define TxQ_IDX_AC30	0x0c
#define TxQ_IDX_AC31	0x0d
#define TxQ_IDX_AC32	0x0e
#define TxQ_IDX_AC33	0x0f

#define TxQ_IDX_ALTX0	0x10
#define TxQ_IDX_BMC0	0x11
#define TxQ_IDX_BCN0	0x12
#define TxQ_IDX_PSMP0	0x13

#define TxQ_IDX_ALTX1	0x14
#define TxQ_IDX_BMC1	0x15
#define TxQ_IDX_BCN1	0x16
#define TxQ_IDX_PSMP1	0x17

#define TxQ_IDX_MCU0	0x00
#define TxQ_IDX_MCU1	0x01
#define TxQ_IDX_MCU2	0x02
#define TxQ_IDX_MCU3	0x03
#define TxQ_IDX_MCU_PDA	0x1e


/* TMAC_TXD_0.p_idx +  TMAC_TXD_0.q_idx */
#define PQ_IDX_WMM0_AC0	((P_IDX_LMAC << 5) | (TxQ_IDX_AC0))
#define PQ_IDX_WMM0_AC1	((P_IDX_LMAC << 5) | (TxQ_IDX_AC1))
#define PQ_IDX_WMM0_AC2	((P_IDX_LMAC << 5) | (TxQ_IDX_AC2))
#define PQ_IDX_WMM0_AC3	((P_IDX_LMAC << 5) | (TxQ_IDX_AC3))
#define PQ_IDX_WMM1_AC0	((P_IDX_LMAC << 5) | (TxQ_IDX_AC10))
#define PQ_IDX_WMM1_AC1	((P_IDX_LMAC << 5) | (TxQ_IDX_AC11))
#define PQ_IDX_WMM1_AC2	((P_IDX_LMAC << 5) | (TxQ_IDX_AC12))
#define PQ_IDX_WMM1_AC3	((P_IDX_LMAC << 5) | (TxQ_IDX_AC13))
#define PQ_IDX_WMM2_AC0	((P_IDX_LMAC << 5) | (TxQ_IDX_AC20))
#define PQ_IDX_WMM2_AC1	((P_IDX_LMAC << 5) | (TxQ_IDX_AC21))
#define PQ_IDX_WMM2_AC2	((P_IDX_LMAC << 5) | (TxQ_IDX_AC22))
#define PQ_IDX_WMM2_AC3	((P_IDX_LMAC << 5) | (TxQ_IDX_AC23))
#define PQ_IDX_WMM3_AC0	((P_IDX_LMAC << 5) | (TxQ_IDX_AC30))
#define PQ_IDX_WMM3_AC1	((P_IDX_LMAC << 5) | (TxQ_IDX_AC31))
#define PQ_IDX_WMM3_AC2	((P_IDX_LMAC << 5) | (TxQ_IDX_AC32))
#define PQ_IDX_WMM3_AC3	((P_IDX_LMAC << 5) | (TxQ_IDX_AC33))

#define PQ_IDX_ALTX0	((P_IDX_LMAC << 5) | (TxQ_IDX_ALTX0))
#define PQ_IDX_BMC0		((P_IDX_LMAC << 5) | (TxQ_IDX_ALTX0))
#define PQ_IDX_BCN0		((P_IDX_LMAC << 5) | (TxQ_IDX_ALTX0))
#define PQ_IDX_PSMP0	((P_IDX_LMAC << 5) | (TxQ_IDX_ALTX0))

#define PQ_IDX_MCU_RQ0		((P_IDX_MCU << 5) | (TxQ_IDX_MCU0))
#define PQ_IDX_MCU_RQ1		((P_IDX_MCU << 5) | (TxQ_IDX_MCU1))
#define PQ_IDX_MCU_RQ2		((P_IDX_MCU << 5) | (TxQ_IDX_MCU2))
#define PQ_IDX_MCU_RQ3		((P_IDX_MCU << 5) | (TxQ_IDX_MCU3))


/******************************************************************************
	TMAC_TXD_1
******************************************************************************/
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TMAC_TXD_1 {
	/* DWORD 1 */
	UINT32 OwnMacAddr : 6;
	UINT32 pkt_ft : 2;
	UINT32 tid : 3;
	UINT32 amsdu : 1;
	UINT32 UNxV : 1;
	UINT32 hdr_pad : 2;
	UINT32 txd_len : 1;
	UINT32 ft : 1;					/* TMI_FT_XXXX */
	UINT32 hdr_format : 2;			/* TMI_HDR_FT_XXX */
	UINT32 hdr_info : 5;			/* in unit of WORD(2 bytes) */
	UINT32 wlan_idx : 8;
} TMAC_TXD_1, *PTMAC_TXD_1;
#else
typedef struct GNU_PACKED _TMAC_TXD_1 {
	/* DWORD 1 */
	UINT32 wlan_idx : 8;
	UINT32 hdr_info : 5;			/* in unit of WORD(2 bytes) */
	UINT32 hdr_format : 2;			/* TMI_HDR_FT_XXX */
	UINT32 ft : 1;			/* TMI_HDR_FT_XXX */
	UINT32 txd_len : 1;					/* TMI_FT_XXXX */
	UINT32 hdr_pad : 2;
	UINT32 UNxV : 1;
	UINT32 amsdu : 1;
	UINT32 tid : 3;
	UINT32 pkt_ft : 2;
	UINT32 OwnMacAddr : 6;
} TMAC_TXD_1, *PTMAC_TXD_1;
#endif

/* TMAC_TXD_1.pkt_ft */
#define	TMI_PKT_FT_HIF_CT		0	/* Cut-through */
#define	TMI_PKT_FT_HIF_SF		1	/* Store & forward  */
#define	TMI_PKT_FT_HIF_CMD	2	/* Command frame to N9/CR4 */
#define	TMI_PKT_FT_HIF_FW		3	/* Firmware frame to PDA */

#define TMI_PKT_FT_MCU_CT       0   /* N9 Cut-through */
#define TMI_PKT_FT_MCU_FW       3   /* N9 to UMAC/LMAC */

/* TMAC_TXD_1.hdr_format */
#define TMI_HDR_FT_NON_80211	0x0
#define TMI_HDR_FT_CMD		0x1
#define TMI_HDR_FT_NOR_80211	0x2
#define TMI_HDR_FT_ENH_80211	0x3

/*
	TMAC_TXD_1.hdr_info
*/
/* if TMAC_TXD_1.hdr_format  == HDR_FORMAT_NON_80211 */
#define TMI_HDR_INFO_0_BIT_MRD		0
#define TMI_HDR_INFO_0_BIT_EOSP		1
#define TMI_HDR_INFO_0_BIT_RMVL		2
#define TMI_HDR_INFO_0_BIT_VLAN		3
#define TMI_HDR_INFO_0_BIT_ETYP		4
#define TMI_HDR_INFO_0_VAL(_mrd, _eosp, _rmvl, _vlan, _etyp)	\
	((((_mrd) ? 1 : 0) << TMI_HDR_INFO_0_BIT_MRD) | \
	 (((_eosp) ? 1 : 0) << TMI_HDR_INFO_0_BIT_EOSP) |\
	 (((_rmvl) ? 1 : 0) << TMI_HDR_INFO_0_BIT_RMVL) |\
	 (((_vlan) ? 1 : 0) << TMI_HDR_INFO_0_BIT_VLAN) |\
	 (((_etyp) ? 1 : 0) << TMI_HDR_INFO_0_BIT_ETYP))


/* if TMAC_TXD_1.hdr_format  == HDR_FORMAT_CMD */
#define TMI_HDR_INFO_1_MASK_RSV		(0x1f)
#define TMI_HDR_INFO_1_VAL			0

/* if TMAC_TXD_1.hdr_format  == HDR_FORMAT_NOR_80211 */
#define TMI_HDR_INFO_2_MASK_LEN	(0x1f)
#define TMI_HDR_INFO_2_VAL(_len)	(_len >> 1)

/* if TMAC_TXD_1.hdr_format  == HDR_FORMAT_ENH_80211 */
#define TMI_HDR_INFO_3_BIT_EOSP	1
#define TMI_HDR_INFO_3_BIT_AMS	2
#define TMI_HDR_INFO_3_VAL(_eosp, _ams)	\
	((((_eosp) ? 1 : 0) << TMI_HDR_INFO_3_BIT_EOSP) | \
	 (((_ams) ? 1 : 0) << TMI_HDR_INFO_3_BIT_AMS))

#define TMI_HDR_INFO_VAL(_fmt, _mrd, _eosp, _rmvl, _vlan, _etype, _len, _ams, _val)	\
	do {\
		switch (_fmt) {\
		case TMI_HDR_FT_NON_80211:\
			(_val) = TMI_HDR_INFO_0_VAL((_mrd), (_eosp), (_rmvl), (_vlan), (_etype));\
			break;\
		case TMI_HDR_FT_CMD:\
			(_val) = TMI_HDR_INFO_1_VAL;\
			break;\
		case TMI_HDR_FT_NOR_80211:\
			ASSERT((((_len) & 1) == 0));\
			(_val) = TMI_HDR_INFO_2_VAL((_len));\
			break;\
		case TMI_HDR_FT_ENH_80211:\
			(_val) = TMI_HDR_INFO_3_VAL((_eosp), (_ams));\
		default:\
			(_val) = 0;\
		} \
	} while (0)


/* TMAC_TXD_1.TxDFmt */
#define TMI_FT_SHORT	0
#define TMI_FT_LONG		1


/* TMAC_TXD_1.HdrPad */
#define TMI_HDR_PAD_BIT_MODE		1
#define TMI_HDR_PAD_MODE_TAIL		0
#define TMI_HDR_PAD_MODE_HEAD	1
#define TMI_HDR_PAD_BIT_LEN	0
#define TMI_HDR_PAD_MASK_LEN	0x3


/******************************************************************************
	TMAC_TXD_2
******************************************************************************/
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TMAC_TXD_2 {
	/* DWORD 2 */
	UINT32 fix_rate : 1;
	UINT32 timing_measure : 1;
	UINT32 ba_disable : 1;
	UINT32 pwr_offset : 5;
	UINT32 max_tx_time : 8;
	UINT32 frag : 2;
	UINT32 htc_vld : 1;
	UINT32 duration : 1;
	UINT32 bip : 1;
	UINT32 bc_mc_pkt : 1;
	UINT32 rts : 1;
	UINT32 sounding : 1;
	UINT32 ndpa : 1;
	UINT32 ndp : 1;
	UINT32 frm_type : 2;
	UINT32 sub_type : 4;
} TMAC_TXD_2, *PTMAC_TXD_2;
#else
typedef struct GNU_PACKED _TMAC_TXD_2 {
	/* DWORD 2 */
	UINT32 sub_type : 4;
	UINT32 frm_type : 2;
	UINT32 ndp : 1;
	UINT32 ndpa : 1;
	UINT32 sounding : 1;
	UINT32 rts : 1;
	UINT32 bc_mc_pkt : 1;
	UINT32 bip : 1;
	UINT32 duration : 1;
	UINT32 htc_vld : 1;
	UINT32 frag : 2;
	UINT32 max_tx_time : 8;
	UINT32 pwr_offset : 5;
	UINT32 ba_disable : 1;
	UINT32 timing_measure : 1;
	UINT32 fix_rate : 1;
} TMAC_TXD_2, *PTMAC_TXD_2;
#endif


/******************************************************************************
	TMAC_TXD_3
******************************************************************************/
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TMAC_TXD_3 {
	/* DWORD 3 */
	UINT32 sn_vld : 1;
	UINT32 pn_vld : 1;
	UINT32 rsv_28_29 : 2;
	UINT32 sn : 12;
	UINT32 remain_tx_cnt : 5;
	UINT32 tx_cnt : 5;
	UINT32 rsv_2_5 : 4;
	UINT32 protect_frm : 1;
	UINT32 no_ack : 1;
} TMAC_TXD_3, *PTMAC_TXD_3;
#else
typedef struct GNU_PACKED _TMAC_TXD_3 {
	/* DWORD 3 */
	UINT32 no_ack : 1;
	UINT32 protect_frm : 1;
	UINT32 rsv_2_5 : 4;
	UINT32 tx_cnt : 5;
	UINT32 remain_tx_cnt : 5;
	UINT32 sn : 12;
	UINT32 rsv_28_29 : 2;
	UINT32 pn_vld : 1;
	UINT32 sn_vld : 1;
} TMAC_TXD_3, *PTMAC_TXD_3;
#endif


/******************************************************************************
	TMAC_TXD_4
******************************************************************************/
typedef struct GNU_PACKED _TMAC_TXD_4 {
	UINT32 pn_low;
} TMAC_TXD_4, *PTMAC_TXD_4;


/******************************************************************************
	TMAC_TXD_5
******************************************************************************/
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TMAC_TXD_5 {
	UINT32 pn_high:16;
	UINT32 rsv_14_15:2;
	UINT32 pwr_mgmt:1;
	UINT32 rsv_12:1;
	UINT32 da_select:1;
	UINT32 tx_status_2_host:1;
	UINT32 tx_status_2_mcu:1;
	UINT32 tx_status_fmt:1;
	UINT32 pid:8;
} TMAC_TXD_5;
#else
typedef struct GNU_PACKED _TMAC_TXD_5 {
	UINT32 pid:8;
	UINT32 tx_status_fmt:1;
	UINT32 tx_status_2_mcu:1;
	UINT32 tx_status_2_host:1;
	UINT32 da_select:1;
	UINT32 rsv_12:1;
	UINT32 pwr_mgmt:1;
	UINT32 rsv_14_15:2;
	UINT32 pn_high:16;
} TMAC_TXD_5, *PTMAC_TXD_5;
#endif

/* TMAC_TXD_5.da_select */
#define TMI_DAS_FROM_MPDU		0
#define TMI_DAS_FROM_WTBL		1

/* TMAC_TXD_5.bar_sn_ctrl */
#define TMI_BSN_CFG_BY_HW	0
#define TMI_BSN_CFG_BY_SW	1

/* TMAC_TXD_5.pwr_mgmt */
#define TMI_PM_BIT_CFG_BY_HW	0
#define TMI_PM_BIT_CFG_BY_SW	1


/******************************************************************************
	TMAC_TXD_6
******************************************************************************/
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TMAC_TXD_6 {
	UINT32 fix_rate_mode:1;
	UINT32 gi:1;
	UINT32 ldpc:1;
	UINT32 TxBF:1;
	UINT32 tx_rate:12;
	UINT32 ant_id:12;
	UINT32 dyn_bw:1;
	UINT32 bw:3;
} TMAC_TXD_6, *PTMAC_TXD_6;
#else
typedef struct GNU_PACKED _TMAC_TXD_6 {
	UINT32 bw:3;
	UINT32 dyn_bw:1;
	UINT32 ant_id:12;
	UINT32 tx_rate:12;
	UINT32 TxBF:1;
	UINT32 ldpc:1;
	UINT32 gi:1;
	UINT32 fix_rate_mode:1;
} TMAC_TXD_6, *PTMAC_TXD_6;
#endif

/* TMAC_TXD_6.fix_rate_mode */
#define TMI_FIX_RATE_BY_TXD	0
#define TMI_FIX_RATE_BY_CR		1

#define TMI_TX_RATE_BIT_STBC		11
#define TMI_TX_RATE_BIT_NSS		9
#define TMI_TX_RATE_MASK_NSS		0x3

#define TMI_TX_RATE_BIT_MODE		6
#define TMI_TX_RATE_MASK_MODE	0x7
#define TMI_TX_RATE_MODE_CCK		0
#define TMI_TX_RATE_MODE_OFDM	1
#define TMI_TX_RATE_MODE_HTMIX	2
#define TMI_TX_RATE_MODE_HTGF		3
#define TMI_TX_RATE_MODE_VHT		4

#define SHORT_PREAMBLE 0
#define LONG_PREAMBLE 1

#define TMI_TX_RATE_BIT_MCS		0
#define TMI_TX_RATE_MASK_MCS		0x3f
#define TMI_TX_RATE_CCK_1M_LP		0
#define TMI_TX_RATE_CCK_2M_LP		1
#define TMI_TX_RATE_CCK_5M_LP		2
#define TMI_TX_RATE_CCK_11M_LP	3

#define TMI_TX_RATE_CCK_2M_SP		5
#define TMI_TX_RATE_CCK_5M_SP		6
#define TMI_TX_RATE_CCK_11M_SP	7

#define TMI_TX_RATE_OFDM_6M		11
#define TMI_TX_RATE_OFDM_9M		15
#define TMI_TX_RATE_OFDM_12M		10
#define TMI_TX_RATE_OFDM_18M		14
#define TMI_TX_RATE_OFDM_24M		9
#define TMI_TX_RATE_OFDM_36M		13
#define TMI_TX_RATE_OFDM_48M		8
#define TMI_TX_RATE_OFDM_54M		12

#define TMI_TX_RATE_HT_MCS0		0
#define TMI_TX_RATE_HT_MCS1		1
#define TMI_TX_RATE_HT_MCS2		2
#define TMI_TX_RATE_HT_MCS3		3
#define TMI_TX_RATE_HT_MCS4		4
#define TMI_TX_RATE_HT_MCS5		5
#define TMI_TX_RATE_HT_MCS6		6
#define TMI_TX_RATE_HT_MCS7		7
#define TMI_TX_RATE_HT_MCS8		8
#define TMI_TX_RATE_HT_MCS9		9
#define TMI_TX_RATE_HT_MCS10		10
#define TMI_TX_RATE_HT_MCS11		11
#define TMI_TX_RATE_HT_MCS12		12
#define TMI_TX_RATE_HT_MCS13		13
#define TMI_TX_RATE_HT_MCS14		14
#define TMI_TX_RATE_HT_MCS15		15
#define TMI_TX_RATE_HT_MCS16		16
#define TMI_TX_RATE_HT_MCS17		17
#define TMI_TX_RATE_HT_MCS18		18
#define TMI_TX_RATE_HT_MCS19		19
#define TMI_TX_RATE_HT_MCS20		20
#define TMI_TX_RATE_HT_MCS21		21
#define TMI_TX_RATE_HT_MCS22		22
#define TMI_TX_RATE_HT_MCS23		23

#define TMI_TX_RATE_HT_MCS32		32

#define TMI_TX_RATE_VHT_MCS0		0
#define TMI_TX_RATE_VHT_MCS1		1
#define TMI_TX_RATE_VHT_MCS2		2
#define TMI_TX_RATE_VHT_MCS3		3
#define TMI_TX_RATE_VHT_MCS4		4
#define TMI_TX_RATE_VHT_MCS5		5
#define TMI_TX_RATE_VHT_MCS6		6
#define TMI_TX_RATE_VHT_MCS7		7
#define TMI_TX_RATE_VHT_MCS8		8
#define TMI_TX_RATE_VHT_MCS9		9


/******************************************************************************
	TMAC_TXD_7
******************************************************************************/
#define CON_SW_TX_TIME_MASK (0x3ff)
#define CON_SW_TX_TIME(p) ((p) & 0x3ff)
#define CON_RSV_10_MASK ((0x1) << 10)
#define CON_RSV_10(p) (((p) & 0x1) << 10)
#define CON_SPE_IDX_MASK ((0x1f) << 11)
#define CON_SPE_IDX(p) (((p) & 0x1f) << 11)
#define CON_PSE_FID_MASK ((0x3fff) << 16)
#define CON_PSE_FID(p) (((p) & 0x3fff) << 16)
#define CON_HW_AMSDU_CAP_MASK ((0x1) << 30)
#define CON_HW_AMSDU_CAP(p) (((p) & 0x1) << 30)
#define CON_HIF_ERR_MASK ((0x1) << 31)
#define CON_HIF_ERR(p) (((p) & 0x1) << 31)

#define SW_TX_TIME_MASK (0xff)
#define SW_TX_TIME(p) (((p) & 0xff))
#define HW_RSV_TX_TIME_MASK ((0x3) << 8)
#define HW_RSV_TX_TIME(p) (((p) & 0x3) << 8)
#define RSV_10_MASK ((0x1) << 10)
#define RSV_10(p) (((p) & 0x1) << 10)
#define SPE_IDX_MASK ((0x1f) << 11)
#define SPE_IDX(p) (((p) & 0x1f) << 11)
#define PP_REF_SUBTYPE_MASK ((0xf) << 16)
#define PP_REF_SUBTYPE(p) (((p) & 0xf) << 16)
#define PP_REF_TYPE_MASK ((0x3) << 20)
#define PP_REF_TYPE(p) (((p) & 0x3) << 20)
#define PSE_FID_MASK ((0x3ff) << 22)
#define PSE_FID(p) (((p) & 0x3ff) << 22)


/******************************************************************************
	TX Descriptor in Long Format (TMAC_TXD_1.FT = 1)
	which including TMAC_TXD_0~ TMAC_TXD_7
******************************************************************************/
typedef struct GNU_PACKED _TMAC_TXD_L {
	TMAC_TXD_0 TxD0;
	TMAC_TXD_1 TxD1;
	TMAC_TXD_2 TxD2;
	TMAC_TXD_3 TxD3;
	TMAC_TXD_4 TxD4;
	TMAC_TXD_5 TxD5;
	TMAC_TXD_6 TxD6;
	UINT32 TxD7;
} TMAC_TXD_L, *PTMAC_TXD_L;


/******************************************************************************
	TX Descriptor in Short Format (TMAC_TXD_1.FT = 0)
	which including TMAC_TXD_0, TMAC_TXD_1, and TMAC_TXD_7
******************************************************************************/
/*
	TX Descriptor in Short Format (TMAC_TXD_1.FT = 0)
*/
typedef struct GNU_PACKED _TMAC_TXD_S {
	TMAC_TXD_0 TxD0;
	TMAC_TXD_1 TxD1;
	UINT32 TxD7;
} TMAC_TXD_S, *PTMAC_TXD_S;

#define NUM_OF_MSDU_ID_IN_TXD   4
#define TXD_MAX_BUF_NUM         4
#define TXD_MSDU_ID_VLD             BIT(15)     /* MSDU valid */

#define TXD_LEN_ML_V2               BIT(15)     /* MSDU last */
#define TXD_LEN_MASK_V2             BITS(0, 11)
#define TXD_LEN_AL                  BIT(15)     /* A-MSDU last */
#define TXD_LEN_ML                  BIT(14)     /* MSDU last */

typedef struct _TXD_PTR_LEN_T {
	UINT32 u4Ptr0;
	UINT16 u2Len0; /* Bit15: AL, Bit14: ML */
	UINT16 u2Len1; /* Bit15: AL, Bit14: ML */
	UINT32 u4Ptr1;
} TXD_PTR_LEN_T, *P_TXD_PTR_LEN_T;

typedef struct _MAC_TX_PKT_T {
	/* DW0 and DW1 */
	UINT16 au2MsduId[NUM_OF_MSDU_ID_IN_TXD];   /* Bit15 indicate valid */

	/* DW2 ~ DW22 */
	TXD_PTR_LEN_T arPtrLen[TXD_MAX_BUF_NUM / 2];

	/* DW31 */
	/* UINT32 u4ReservedTail; */
} MAC_TX_PKT_T, *P_MAC_TX_PKT_T;

#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXP_MSDU_INFO {
	UINT32 msdu_vld_3:1;
	UINT32 msdu_id_3:15;
	UINT32 msdu_vld_2:1;
	UINT32 msdu_id_2:15;
	UINT32 msdu_vld_1:1;
	UINT32 msdu_id_1:15;
	UINT32 msdu_vld_0:1;
	UINT32 msdu_id_0:15;
} TXP_MSDU_INFO;
#else
typedef struct GNU_PACKED _TXP_MSDU_INFO {
	UINT32 msdu_id_0:15;
	UINT32 msdu_vld_0:1;

	UINT32 msdu_id_1:15;
	UINT32 msdu_vld_1:1;

	UINT32 msdu_id_2:15;
	UINT32 msdu_vld_2:1;

	UINT32 msdu_id_3:15;
	UINT32 msdu_vld_3:1;
} TXP_MSDU_INFO;
#endif /* RT_BIG_ENDIAN */


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXP_MSDU_PTR {
	UINT32 txp_addr_1;
	UINT32 amsdu_last_1:1;
	UINT32 msdu_last_1:1;
	UINT32 src_1:1;
	UINT32 tx_len_1:13;
	UINT32 amsdu_last_0:1;
	UINT32 msdu_last_0:1;
	UINT32 src_0:1;
	UINT32 tx_len_0:13;
	UINT32 txp_addr_0;
} TXP_MSDU_PTR;
#else
typedef struct GNU_PACKED _TXP_MSDU_PTR {
	UINT32 txp_addr_0;

	UINT32 tx_len_0:13;
	UINT32 src_0:1;
	UINT32 msdu_last_0:1;
	UINT32 amsdu_last_0:1;
	UINT32 tx_len_1:13;
	UINT32 src_1:1;
	UINT32 msdu_last_1:1;
	UINT32 amsdu_last_1:1;

	UINT32 txp_addr_1;
} TXP_MSDU_PTR;
#endif /* RT_BIG_ENDIAN */


/* TxP Structure */
typedef struct GNU_PACKED _TMAC_TXP_S {
	struct _TXP_MSDU_INFO msdu_info;
	struct _TXP_MSDU_PTR txp_list_01;
	struct _TXP_MSDU_PTR txp_list_23;
} TMAC_TXP_S;


typedef struct GNU_PACKED _TMAC_TXP_L {
	struct _TXP_MSDU_INFO msdu_info;
	struct _TXP_MSDU_PTR txp_list_01;
	struct _TXP_MSDU_PTR txp_list_23;
	struct _TXP_MSDU_PTR txp_list_45;
	struct _TXP_MSDU_PTR txp_list_67;
	struct _TXP_MSDU_PTR txp_list_89;
	struct _TXP_MSDU_PTR txp_list_1011;
	struct _TXP_MSDU_PTR txp_list_1213;
} TMAC_TXP_L;


#define MAX_BUF_NUM_PER_PKT	6
#define CUT_THROUGH_PAYLOAD_LEN	72
/* Cut thorugh long format */
typedef struct _CUT_TH_TXINFO_S {
	/* DW0 ~ DW3 */
	TMAC_TXD_S txd_s;

	UINT16	u2TypeAndFlags;  /* Bit0:  cr4 refer to TXD of host*/
	UINT16	u2MsduToken;
	UINT8	ucBssIndex;
	UINT8	aucReserved[2];
	UINT8	ucBufNum;

	UINT32	au4BufPtr[MAX_BUF_NUM_PER_PKT];
	UINT16	au2BufLen[MAX_BUF_NUM_PER_PKT];
} CUT_TH_TXINFO_S;


/* Cut thorugh long format */
typedef  struct _CUT_TH_TXINFO_L {
	/* DW0 ~ DW7 */
	TMAC_TXD_L	txd_l;

	UINT16         u2TypeAndFlags; /* Bit0:  cr4 refer to TXD of host */
	UINT16         u2MsduToken;
	UINT8          ucBssIndex;
	UINT8          aucReserved[2];
	UINT8          ucBufNum;

	UINT32         au4BufPtr[MAX_BUF_NUM_PER_PKT];
	UINT16         au2BufLen[MAX_BUF_NUM_PER_PKT];
} CUT_TH_TXINFO_L;



#define RMAC_RXD_0_PKT_TYPE_MASK		0xe0000000
#define RMAC_RX_PKT_TYPE_RX_TXS			0x00
#define RMAC_RX_PKT_TYPE_RX_TXRXV		0x01
#define RMAC_RX_PKT_TYPE_RX_NORMAL		0x02
#define RMAC_RX_PKT_TYPE_RX_DUP_RFB		0x03
#define RMAC_RX_PKT_TYPE_RX_TMR			0x04
#define RMAC_RX_PKT_TYPE_RETRIEVE		0x05
#ifdef CUT_THROUGH
#define RMAC_RX_PKT_TYPE_TXRX_NOTIFY    0x06
#endif /* CUT_THROUGH */
#define RMAC_RX_PKT_TYPE_RX_EVENT		0x07
#define RMAC_RX_PKT_TYPE(_x)			(((_x) & RMAC_RXD_0_PKT_TYPE_MASK) >> 29)
#define RMAC_RXD_0_PKT_RX_BYTE_COUNT_MASK 0x0000ffff
#define RMAC_RX_PKT_RX_BYTE_COUNT(_x)	(((_x) & RMAC_RXD_0_PKT_RX_BYTE_COUNT_MASK))

#ifdef RT_BIG_ENDIAN

#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RMAC_RXD_0_TXS {
	UINT32 TxSPktType:3;
	UINT32 TxSRsv:8;
	UINT32 TxSCnt:5;
	UINT32 TxSRxByteCnt:16;
} RMAC_RXD_0_TXS, *PRMAC_RXD_0_TXS;
#else
typedef struct GNU_PACKED _RMAC_RXD_0_TXS {
	UINT32 TxSRxByteCnt:16;
	UINT32 TxSCnt:5;
	UINT32 TxSRsv:8;
	UINT32 TxSPktType:3;
} RMAC_RXD_0_TXS, *PRMAC_RXD_0_TXS;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RMAC_RXD_0_TXRXV {
	UINT32 PktType:3;
	UINT32 Rsv:8;
	UINT32 RxvCnt:5;
	UINT32 RxByteCnt:16;
} RMAC_RXD_0_TXRXV, *PRMAC_RXD_0_TXRXV;
#else
typedef struct GNU_PACKED _RMAC_RXD_0_TXRXV {
	UINT32 RxByteCnt:16;
	UINT32 RxvCnt:5;
	UINT32 Rsv:8;
	UINT32 PktType:3;
} RMAC_RXD_0_TXRXV, *PRMAC_RXD_0_TXRXV;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RMAC_RXD_0 {
	UINT32 PktType:3;
	UINT32 RfbGroupVld:4;
	UINT32 UdpTcpChkSumOffload:1;
	UINT32 IpChkSumOffload:1;
	UINT32 EthTypeOffset:7;
	UINT32 RxByteCnt:16;
} RMAC_RXD_0, *PRMAC_RXD_0;
#else
typedef struct GNU_PACKED _RMAC_RXD_0 {
	UINT32 RxByteCnt:16;
	UINT32 EthTypeOffset:7;
	UINT32 IpChkSumOffload:1;
	UINT32 UdpTcpChkSumOffload:1;
	UINT32 RfbGroupVld:4;
	UINT32 PktType:3;
} RMAC_RXD_0, *PRMAC_RXD_0;
#endif
/* ---UNIFIED_TXD_RXD */


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RMAC_RXD_1 {
	UINT32   RxDBssidIdx:6;
	UINT32   PayloadFmt:2;
	UINT32   HdrTranslation:1;
	UINT32   HdrOffset:1;
	UINT32   MacHdrLen:6;
	UINT32   ChFreq:8;
	UINT32   KeyId:2;
	UINT32   BcnWithUCast:1;
	UINT32   BcnWithBMcst:1;
	UINT32   BF_RPT:1;
	UINT32   a1_type:2;
	UINT32   HTC:1;
} RMAC_RXD_1, *PRMAC_RXD_1;
#else
typedef struct GNU_PACKED _RMAC_RXD_1 {
	UINT32   HTC:1;
	UINT32   a1_type:2;
	UINT32   BF_RPT:1;
	UINT32   BcnWithBMcst:1;
	UINT32   BcnWithUCast:1;
	UINT32   KeyId:2;
	UINT32   ChFreq:8;
	UINT32   MacHdrLen:6;
	UINT32   HdrOffset:1;
	UINT32   HdrTranslation:1;
	UINT32   PayloadFmt:2;
	UINT32   RxDBssidIdx:6;
} RMAC_RXD_1, *PRMAC_RXD_1;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RMAC_RXD_2 {
	UINT32   NonAmpduFrm:1;
	UINT32   NonAmpduSfrm:1;
	UINT32   NonDataFrm:1;
	UINT32   NullFrm:1;
	UINT32   FragFrm:1;
	UINT32   InterestedFrm:1;
	UINT32   HdrTransFail:1;
	UINT32   ExMaxRxLen:1;
	UINT32   DeAmsduFail:1;
	UINT32   LenMis:1;
	UINT32   TkipMicErr:1;
	UINT32   IcvErr:1;
	UINT32   CipherLenMis:1;
	UINT32   CipherMis:1;
	UINT32   FcsErr:1;
	UINT32   SwBit:1;
	UINT32   SecMode:4;
	UINT32   RxDTid:4;
	UINT32   RxDWlanIdx:8;
} RMAC_RXD_2, *PRMAC_RXD_2;
#else
typedef struct GNU_PACKED _RMAC_RXD_2 {
	UINT32   RxDWlanIdx:8;
	UINT32   RxDTid:4;
	UINT32   SecMode:4;
	UINT32   SwBit:1;
	UINT32   FcsErr:1;
	UINT32   CipherMis:1;
	UINT32   CipherLenMis:1;
	UINT32   IcvErr:1;
	UINT32   TkipMicErr:1;
	UINT32   LenMis:1;
	UINT32   DeAmsduFail:1;
	UINT32   ExMaxRxLen:1;
	UINT32   HdrTransFail:1;
	UINT32   InterestedFrm:1;
	UINT32   FragFrm:1;
	UINT32   NullFrm:1;
	UINT32   NonDataFrm:1;
	UINT32   NonAmpduSfrm:1;
	UINT32   NonAmpduFrm:1;
} RMAC_RXD_2, *PRMAC_RXD_2;
#endif
/* ---UNIFIED_TXD_RXD */

/*//do unify
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED rmac_rxd_3_normal {
	UINT32 pf_sts:2;
	UINT32 pf_mode:1;
	UINT32 cls_bitmap:10;
	UINT32 wol:5;
	UINT32 magic_pkt:1;
	UINT32 OFLD:2;
	UINT32 cls:1;
	UINT32 pattern_drop_bit:1;
	UINT32 tsf_compare_loss:1;
	UINT32 rx_vector_seq:8;
} RMAC_RXD_3_NORMAL;
#else
typedef struct GNU_PACKED rmac_rxd_3_normal {
	UINT32 rx_vector_seq:8;
	UINT32 tsf_compare_loss:1;
	UINT32 pattern_drop_bit:1;
	UINT32 cls:1;
	UINT32 OFLD:2;
	UINT32 magic_pkt:1;
	UINT32 wol:5;
	UINT32 cls_bitmap:10;
	UINT32 pf_mode:1;
	UINT32 pf_sts:2;
} RMAC_RXD_3_NORMAL;
#endif
*/


/* +++UNIFIED_TXD_RXD */
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RMAC_RXD_0_TMR {
	UINT32 PktType:3;
	UINT32 IR:1;
	UINT32 TodVld:1;
	UINT32 ToaVld:1;
	UINT32 SubType:4;
	UINT32 Type:2;
	UINT32 Tmf:1;
	UINT32 Nc:1;
	UINT32 TxVFrmMode:2;
	UINT32 RxByteCnt:16;
} RMAC_RXD_0_TMR, *PRMAC_RXD_0_TMR;
#else
typedef struct GNU_PACKED _RMAC_RXD_0_TMR {
	UINT32 RxByteCnt:16;
	UINT32 TxVFrmMode:2;
	UINT32 Nc:1;
	UINT32 Tmf:1;
	UINT32 Type:2;
	UINT32 SubType:4;
	UINT32 ToaVld:1;
	UINT32 TodVld:1;
	UINT32 IR:1;
	UINT32 PktType:3;
} RMAC_RXD_0_TMR, *PRMAC_RXD_0_TMR;
#endif
/* ---UNIFIED_TXD_RXD */

typedef union GNU_PACKED _RMAC_RXD_0_UNION {
	struct _RMAC_RXD_0_TXS TxSD0;
	struct _RMAC_RXD_0 RxD0;
	struct _RMAC_RXD_0_TXRXV RxvD0;
	struct _RMAC_RXD_0_TMR TmrD0;
	UINT32 word;
} RMAC_RXD_0_UNION, *PRMAC_RXD_0_UNION;

/* +++UNIFIED_TXD_RXD */
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RMAC_RXD_3 {
	UINT32   PfStatus:2;
	UINT32   PfMode:1;
	UINT32   ClsBitMap:10;
	UINT32   WoWlan:5;
	UINT32   MgcPkt:1;
	UINT32   OfldPf:2;
	UINT32   ClsPkt:1;
	UINT32   Drop:1;
	UINT32   Tcl:1;
	UINT32   RxVSeq:8;
} RMAC_RXD_3, *PRMAC_RXD_3;
#else
typedef struct GNU_PACKED _RMAC_RXD_3 {
	UINT32   RxVSeq:8;
	UINT32   Tcl:1;
	UINT32   Drop:1;
	UINT32   ClsPkt:1;
	UINT32   OfldPf:2;
	UINT32   MgcPkt:1;
	UINT32   WoWlan:5;
	UINT32   ClsBitMap:10;
	UINT32   PfMode:1;
	UINT32   PfStatus:2;
} RMAC_RXD_3, *PRMAC_RXD_3;
#endif
/* ---UNIFIED_TXD_RXD */


/* +++UNIFIED_TXD_RXD */
/*
	Rx Normal Data frame Format
*/
typedef struct GNU_PACKED _RXD_BASE_STRUCT {
	/* DWORD 0 */
	struct _RMAC_RXD_0 RxD0;
	/* DWORD 1 */
	struct _RMAC_RXD_1 RxD1;
	/* DWORD 2 */
	struct _RMAC_RXD_2 RxD2;
	/* DWORD 3 */
	/* struct rmac_rxd_3_normal RxD3;//do unify */
	struct _RMAC_RXD_3 RxD3;
} RXD_BASE_STRUCT, *PRXD_BASE_STRUCT;
/* ---UNIFIED_TXD_RXD */


#define RXS_GROUP1 (1 << 0)
#define RXS_GROUP2 (1 << 1)
#define RXS_GROUP3 (1 << 2)
#define RXS_GROUP4 (1 << 3)

#define RMAC_INFO_BASE_SIZE	16
#define RMAC_INFO_GRP_1_SIZE    16
#define RMAC_INFO_GRP_2_SIZE    8
#define RMAC_INFO_GRP_3_SIZE    24
#define RMAC_INFO_GRP_4_SIZE    16


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RMAC_RXD_4 {

	/* DW4 */
	UINT32   TA0:16;
	UINT32   FrmCtrl:16;
} RMAC_RXD_4, *PRMAC_RXD_4;
#else
typedef struct GNU_PACKED _RMAC_RXD_4 {

	/* DW4 */
	UINT32   FrmCtrl:16;
	UINT32   TA0:16;
} RMAC_RXD_4, *PRMAC_RXD_4;
#endif



typedef struct GNU_PACKED _RMAC_RXD_5 {
	UINT32   TA1;
} RMAC_RXD_5, *PRMAC_RXD_5;


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RMAC_RXD_6 {
	UINT32   QoS:16;
	UINT32   Seq:12;
	UINT32   Frag:4;
} RMAC_RXD_6, *PRMAC_RXD_6;
#else
typedef struct GNU_PACKED _RMAC_RXD_6 {
	UINT32   Frag:4;
	UINT32   Seq:12;
	UINT32   QoS:16;
} RMAC_RXD_6, *PRMAC_RXD_6;
#endif



typedef struct GNU_PACKED _RMAC_RXD_7 {
	UINT32   HTCtrl;
} RMAC_RXD_7, *PRMAC_RXD_7;


typedef struct GNU_PACKED _RXD_GRP4_STRUCT {
	/* DWORD 4 */
	struct _RMAC_RXD_4 rxd_4;

	/* DWORD 5 */
	struct _RMAC_RXD_5 rxd_5;

	/* DWORD 6 */
	struct _RMAC_RXD_6 rxd_6;

	/* DWORD 7 */
	struct _RMAC_RXD_7 rxd_7;
} RXD_GRP4_STRUCT;


typedef union GNU_PACKED _RMAC_RXD_8 {
	UINT32   RscPn0;
	UINT32   WPIPn0;
	UINT32   Word;
} RMAC_RXD_8, *PRMAC_RXD_8;


#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _RMAC_RXD_9 {
	struct {
		UINT32 Rsv:16;
		UINT32 PN:16;
	} RscPn1;
	UINT32   WPIPn1;
	UINT32 Word;
} RMAC_RXD_9, *PRMAC_RXD_9;
#else
typedef union GNU_PACKED _RMAC_RXD_9 {
	struct {
		UINT32 PN:16;
		UINT32 Rsv:16;
	} RscPn1;
	UINT32   WPIPn1;
	UINT32 Word;
} RMAC_RXD_9, *PRMAC_RXD_9;
#endif


typedef struct GNU_PACKED _RMAC_RXD_10 {
	UINT32   WpiPn2;
} RMAC_RXD_10, *PRMAC_RXD_10;


typedef struct GNU_PACKED _RMAC_RXD_11 {
	UINT32   WpiPn3;
} RMAC_RXD_11, *PRMAC_RXD_11;


typedef struct GNU_PACKED _RXD_GRP1_STRUCT {
	/* DWORD 8 */
	union  _RMAC_RXD_8 rxd_8;

	/* DWORD 9 */
	union  _RMAC_RXD_9 rxd_9;

	/* DWORD 10 */
	struct  _RMAC_RXD_10 rxd_10;

	/* DWORD 11 */
	struct  _RMAC_RXD_11 rxd_11;
} RXD_GRP1_STRUCT;


typedef struct GNU_PACKED _RMAC_RXD_12 {
	UINT32   Timestamp;
} RMAC_RXD_12, *PRMAC_RXD_12;


typedef struct GNU_PACKED _RMAC_RXD_13 {
	UINT32   Crc;
} RMAC_RXD_13, *PRMAC_RXD_13;


typedef struct GNU_PACKED _RXD_GRP2_STRUCT {
	/* DWORD 12 */
	struct _RMAC_RXD_12 rxd_12;

	/* DWORD 13 */
	struct _RMAC_RXD_13 rxd_13;
} RXD_GRP2_STRUCT;


typedef struct GNU_PACKED _RMAC_RXD_14 {
	UINT32   RxVCycle1;
} RMAC_RXD_14, *PRMAC_RXD_14;


typedef struct GNU_PACKED _RMAC_RXD_15 {
	UINT32   RxVCycle2;
} RMAC_RXD_15, *PRMAC_RXD_15;


typedef struct GNU_PACKED _RMAC_RXD_16 {
	UINT32   RxVCycle3;
} RMAC_RXD_16, *PRMAC_RXD_16;


typedef struct GNU_PACKED _RMAC_RXD_17 {
	UINT32   RxVCycle4;
} RMAC_RXD_17, *PRMAC_RXD_17;


typedef struct GNU_PACKED _RMAC_RXD_18 {
	UINT32   RxVCycle5;
} RMAC_RXD_18, *PRMAC_RXD_18;


typedef struct GNU_PACKED _RMAC_RXD_19 {
	UINT32   RxVCycle6;
} RMAC_RXD_19, *PRMAC_RXD_19;


typedef struct GNU_PACKED _RXD_GRP3_STRUCT {
	/* DWORD 13 */
	struct _RMAC_RXD_13 rxd_13;

	/* DWORD 14 */
	struct _RMAC_RXD_14 rxd_14;

	/* DWORD 15 */
	struct _RMAC_RXD_15 rxd_15;

	/* DWORD 16 */
	struct _RMAC_RXD_16 rxd_16;

	/* DWORD 17 */
	struct _RMAC_RXD_17 rxd_17;

	/* DWORD 18 */
	struct _RMAC_RXD_18 rxd_18;

	/* DWORD 19 */
	struct _RMAC_RXD_19 rxd_19;

} RXD_GRP3_STRUCT;



#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_CSO_STRUCT {
	UINT32 Rsv11to31:21;
	UINT32 UnknownNextHdr:1;
	UINT32 IpFrag:1;
	UINT32 IpLenMismatch:1;
	UINT32 ChksumType:4;
	UINT32 ChksumStatus:4;
} RX_CSO_STRUCT;
#else
typedef struct GNU_PACKED _RX_CSO_STRUCT {
	UINT32 ChksumStatus:4;
	UINT32 ChksumType:4;
	UINT32 IpLenMismatch:1;
	UINT32 IpFrag:1;
	UINT32 UnknownNextHdr:1;
	UINT32 Rsv11to31:21;
} RX_CSO_STRUCT;
#endif


/******************************************************************************
	LMAC TxS Structure

	TXS_D_0
	TXS_D_1
	TXS_D_2
	TXS_D_3
	TXS_D_4
	TXS_D_5
	TXS_D_6
******************************************************************************/

/******************************************************************************
	TXS_D_0
******************************************************************************/
/* TX Status Report Format */
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXS_D_0 {
	UINT32 TxS_PId:8;
	UINT32 Rsv23:1;
	UINT32 BAFail:1;
	UINT32 PSBit:1;
	UINT32 TxOp:1;
	UINT32 BE:1;
	UINT32 LE:1;
	UINT32 RE:1;
	UINT32 ME:1;
	UINT32 TxS2H:1;
	UINT32 TxS2M:1;
	UINT32 TxSFmt:1;
	UINT32 TxS_FR:1;
	UINT32 TxRate:12;
} TXS_D_0, *PTXS_D_0;
#else
typedef struct GNU_PACKED _TXS_D_0 {
	UINT32 TxRate:12;
	UINT32 TxS_FR:1;
	UINT32 TxSFmt:1;
	UINT32 TxS2M:1;
	UINT32 TxS2H:1;
	UINT32 ME:1;
	UINT32 RE:1;
	UINT32 LE:1;
	UINT32 BE:1;
	UINT32 TxOp:1;
	UINT32 PSBit:1;
	UINT32 BAFail:1;
	UINT32 Rsv23:1;
	UINT32 TxS_PId:8;
} TXS_D_0, *PTXS_D_0;
#endif


/******************************************************************************
	TXS_D_1
******************************************************************************/
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXS_D_1 {
	UINT32 TxS_AntId:12;
	UINT32 TxS_RespRate:4;
	UINT32 TxS_TxBW:2;
	UINT32 TxS_ITxBF:1;
	UINT32 TxS_ETxBF:1;
	UINT32 TxS_Tid:3;
	UINT32 TxS_AMPDU:1;
	UINT32 TxS_FianlMPDU:1;
	UINT32 TxPwrdBm:7;
} TXS_D_1, *PTXS_D_1;
#else
typedef struct GNU_PACKED _TXS_D_1 {
	UINT32 TxPwrdBm:7;
	UINT32 TxS_FianlMPDU:1;
	UINT32 TxS_AMPDU:1;
	UINT32 TxS_Tid:3;
	UINT32 TxS_ETxBF:1;
	UINT32 TxS_ITxBF:1;
	UINT32 TxS_TxBW:2;
	UINT32 TxS_RespRate:4;
	UINT32 TxS_AntId:12;
} TXS_D_1, *PTXS_D_1;
#endif


/******************************************************************************
	TXS_D_2
******************************************************************************/
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXS_D_2 {
	UINT32 TxS_WlanIdx:8;
	UINT32 TxS_RxVSN:8;
	UINT32 TxS_TxDelay:16;
} TXS_D_2, *PTXS_D_2;
#else
typedef struct GNU_PACKED _TXS_D_2 {
	UINT32 TxS_TxDelay:16;
	UINT32 TxS_RxVSN:8;
	UINT32 TxS_WlanIdx:8;
} TXS_D_2, *PTXS_D_2;
#endif


/******************************************************************************
	TXS_D_3
******************************************************************************/
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _TXS_D_3 {
	struct {
		UINT32 TxS_LastTxRateIdx:3;
		UINT32 TxS_MpduTxCnt:5;
		UINT32 TxS_Rsv:12;
		UINT32 TxS_SN:12;
	} type_0;

	struct {
		UINT32 TxS_LastTxRateIdx:3;
		UINT32 TxS_MpduTxCnt:5;
		UINT32 TxS_TSSI1:12;
		UINT32 TxS_TSSI0:12;
	} type_1;

	UINT32 word;
} TXS_D_3, *PTXS_D_3;
#else
typedef union GNU_PACKED _TXS_D_3 {
	struct {
		UINT32 TxS_SN:12;
		UINT32 TxS_Rsv:12;
		UINT32 TxS_MpduTxCnt:5;
		UINT32 TxS_LastTxRateIdx:3;
	} type_0;

	struct {
		UINT32 TxS_TSSI0:12;
		UINT32 TxS_TSSI1:12;
		UINT32 TxS_MpduTxCnt:5;
		UINT32 TxS_LastTxRateIdx:3;
	} type_1;

	UINT32 word;
} TXS_D_3, *PTXS_D_3;
#endif


/******************************************************************************
	TXS_D_4
******************************************************************************/
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _TXS_D_4 {
	struct {
		UINT32 TimeStamp;
	} type_0;

	struct {
		UINT32 TxS_Rsv:8;
		UINT32 TxS_TSSI3:12;
		UINT32 TxS_TSSI2:12;
	} type_1;

	UINT32 word;
} TXS_D_4, *PTXS_D_4;
#else
typedef union GNU_PACKED _TXS_D_4 {
	struct {
		UINT32 TimeStamp;
	} type_0;

	struct {
		UINT32 TxS_TSSI2:12;
		UINT32 TxS_TSSI3:12;
		UINT32 TxS_Rsv:8;
	} type_1;

	UINT32 word;
} TXS_D_4, *PTXS_D_4;
#endif


/******************************************************************************
	TXS_D_5
******************************************************************************/
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _TXS_D_5 {
	struct {
		UINT32 TxS_Rsv:7;
		UINT32 TxS_FrontTime:25;
	} type_0;

	struct {
		UINT32 TxS_Noise3:8;
		UINT32 TxS_Noise2:8;
		UINT32 TxS_Noise1:8;
		UINT32 TxS_Noise0:8;
	} type_1;

	UINT32 word;
} TXS_D_5, *PTXS_D_5;
#else
typedef union GNU_PACKED _TXS_D_5 {
	struct {
		UINT32 TxS_FrontTime:25;
		UINT32 TxS_Rsv:7;
	} type_0;

	struct {
		UINT32 TxS_Noise0:8;
		UINT32 TxS_Noise1:8;
		UINT32 TxS_Noise2:8;
		UINT32 TxS_Noise3:8;
	} type_1;

	UINT32 word;
} TXS_D_5, *PTXS_D_5;
#endif


/* TODO: shiang-MT7615, not mofiy yet for DW6~DW6 !! */
/******************************************************************************
	TXS_D_6
******************************************************************************/
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _TXS_D_6 {
	struct {
		UINT32 TxS_Rsv;
	} type_0;

	struct {
		UINT32 RCPI3:8;
		UINT32 RCPI2:8;
		UINT32 RCPI1:8;
		UINT32 RCPI0:8;
	} type_1;

	UINT32 Word;
} TXS_D_6, *PTXS_D_6;
#else
typedef union GNU_PACKED _TXS_D_6 {
	struct {
		UINT32 TxS_Rsv;
	} type_0;

	struct {
		UINT32 RCPI0:8;
		UINT32 RCPI1:8;
		UINT32 RCPI2:8;
		UINT32 RCPI3:8;
	} type_1;

	UINT32 Word;
} TXS_D_6, *PTXS_D_6;
#endif /* RT_BIG_ENDIAN */


/******************************************************************************
	TXS_STRUC
******************************************************************************/
typedef struct GNU_PACKED _TXS_STRUC {
	TXS_D_0 TxSD0;
	TXS_D_1 TxSD1;
	TXS_D_2 TxSD2;
	TXS_D_3 TxSD3;
	TXS_D_4 TxSD4;
	TXS_D_5 TxSD5;
	TXS_D_6 TxSD6;
} TXS_STRUC;


/******************************************************************************
	TXS Frame Format
******************************************************************************/
typedef struct GNU_PACKED _TXS_FRM_STRUC {
	struct _RMAC_RXD_0_TXS TxSD0;
	TXS_STRUC txs_info[0];
} TXS_FRM_STRUC;




#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR1_1ST_CYCLE {
	UINT32 ACID_DET_UPPER:1;
	UINT32 ACID_DET_LOWER:1;
	UINT32 VHTA2_B8_B3:6;
	UINT32 NumRx:2;
	UINT32 HtNoSound:1;
	UINT32 HtSmooth:1;
	UINT32 HtShortGi:1;
	UINT32 HtAggregation:1;
	UINT32 VHTA1_B22:1;
	UINT32 FrMode:2;
	UINT32 TxMode:3;
	UINT32 HtExtltf:2;
	UINT32 HtAdCode:1;
	UINT32 HtStbc:2;
	UINT32 TxRate:7;
} RX_VECTOR1_1ST_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR1_1ST_CYCLE {
	UINT32 TxRate:7;
	UINT32 HtStbc:2;
	UINT32 HtAdCode:1; /*0: BCC, 1:LDPC */
	UINT32 HtExtltf:2;
	UINT32 TxMode:3;
	UINT32 FrMode:2;
	UINT32 VHTA1_B22:1;
	UINT32 HtAggregation:1;
	UINT32 HtShortGi:1;
	UINT32 HtSmooth:1;
	UINT32 HtNoSound:1;
	UINT32 NumRx:2;
	UINT32 VHTA2_B8_B3:6;
	UINT32 ACID_DET_LOWER:1;
	UINT32 ACID_DET_UPPER:1;
} RX_VECTOR1_1ST_CYCLE;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR1_2ND_CYCLE {
	UINT32 SelAnt:1;
	UINT32 RxValidIndicator:1;
	UINT32 NstsField:3;
	UINT32 GroupId:6;
	UINT32 Length:21;
} RX_VECTOR1_2ND_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR1_2ND_CYCLE {
	UINT32 Length:21;
	UINT32 GroupId:6;
	UINT32 NstsField:3;
	UINT32 RxValidIndicator:1;
	UINT32 SelAnt:1;
} RX_VECTOR1_2ND_CYCLE;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR1_3TH_CYCLE {
	UINT32 WBRssiRx:8;
	UINT32 IBRssiRx:8;
	UINT32 FgacCalLnaRx:2;
	UINT32 POPEverTrig:1;
	UINT32 BFAgcApply:1;
	UINT32 VHTA1_B21_B10:12;
} RX_VECTOR1_3TH_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR1_3TH_CYCLE {
	UINT32 VHTA1_B21_B10:12;
	UINT32 BFAgcApply:1;
	UINT32 POPEverTrig:1;
	UINT32 FgacCalLnaRx:2;
	UINT32 IBRssiRx:8;
	UINT32 WBRssiRx:8;
} RX_VECTOR1_3TH_CYCLE;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR1_4TH_CYCLE {
	UINT32 RCPI3:8;
	UINT32 RCPI2:8;
	UINT32 RCPI1:8;
	UINT32 RCPI0:8;
} RX_VECTOR1_4TH_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR1_4TH_CYCLE {
	UINT32 RCPI0:8;
	UINT32 RCPI1:8;
	UINT32 RCPI2:8;
	UINT32 RCPI3:8;
} RX_VECTOR1_4TH_CYCLE;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR1_5TH_CYCLE {
	UINT32 MISC1:25;
	UINT32 NoiseBalanceEnable:1;
	UINT32 FagcLpfGainx:4;
	UINT32 FagcLnaGainx:2;
} RX_VECTOR1_5TH_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR1_5TH_CYCLE_MISC_OFDM {
	UINT32 OfdmFoe:12;
	UINT32 LtfProcTime:7;
	UINT32 rx_ce_ltf_snr:6;
} RX_VECTOR1_5TH_CYCLE_MISC_OFDM;

typedef struct GNU_PACKED _RX_VECTOR1_5TH_CYCLE_MISC_CCK {
	UINT32 CckFoe:11;
	UINT32 InitDfeLmsErr:10;
	UINT32 MRC:2;
	UINT32 AntSelectR0_R1:2;
} RX_VECTOR1_5TH_CYCLE_MISC_CCK;

typedef struct GNU_PACKED _RX_VECTOR1_5TH_CYCLE {
	UINT32 FagcLnaGainx:2;
	UINT32 FagcLpfGainx:4;
	UINT32 NoiseBalanceEnable:1;
	UINT32 MISC1:25;
} RX_VECTOR1_5TH_CYCLE;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR1_6TH_CYCLE {
	UINT32 Nf3:8;
	UINT32 Nf2:8;
	UINT32 Nf1:8;
	UINT32 Nf0:8;
} RX_VECTOR1_6TH_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR1_6TH_CYCLE {
	UINT32 Nf0:8;
	UINT32 Nf1:8;
	UINT32 Nf2:8;
	UINT32 Nf3:8;
} RX_VECTOR1_6TH_CYCLE;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR2_1ST_CYCLE {
	UINT32 FCSErr:1;
	UINT32 RxCERmsd:3;
	UINT32 VHT_A2:2;
	UINT32 ScrambleSeed:3;
	UINT32 BtEnv:1;
	UINT32 RxLQ:14;
	UINT32 Sec80ItfrEnv:1;
	UINT32 Sec40ItfrEnv:1;
	UINT32 SecItfrEnv:1;
	UINT32 PrimItfrEnv:1;
} RX_VECTOR2_1ST_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR2_1ST_CYCLE {
	UINT32 PrimItfrEnv:1;
	UINT32 SecItfrEnv:1;
	UINT32 Sec40ItfrEnv:1;
	UINT32 Sec80ItfrEnv:1;
	UINT32 RxLQ:14;
	UINT32 BtEnv:1;
	UINT32 ScrambleSeed:3;
	UINT32 VHT_A2:2;
	UINT32 RxCERmsd:3;
	UINT32 FCSErr:1;
} RX_VECTOR2_1ST_CYCLE;
#endif

#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR2_2ND_CYCLE {
	UINT32 WBRssi0:8;
	UINT32 IBRssi0:8;
	UINT32 CagcSTFDet:3;
	UINT32 HtSTFDet:1;
	UINT32 PostDewSecCh:1;
	UINT32 PostBWDSecCh:2;
	UINT32 RxBW:2;
	UINT32 PostTMD:7;
} RX_VECTOR2_2ND_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR2_2ND_CYCLE {
	UINT32 PostTMD:7;
	UINT32 RxBW:2;
	UINT32 PostBWDSecCh:2;
	UINT32 PostDewSecCh:1;
	UINT32 HtSTFDet:1;
	UINT32 CagcSTFDet:3;
	UINT32 IBRssi0:8;
	UINT32 WBRssi0:8;
} RX_VECTOR2_2ND_CYCLE;
#endif

#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR2_3TH_CYCLE {
	UINT32 FcsErr:1;
	UINT32 OfdmDewModeDet:1;
	UINT32 OfdmCeLtfComb:1;
	UINT32 OfdmCeGicEnb:1;
	UINT32 OfdmCeRmsdId:3;
	UINT32 Reserved1:1;
	UINT32 BgAgcWbRssix:8;
	UINT32 BfAgcIbRssix:8;
	UINT32 BfAgcLpfGain:4;
	UINT32 Reserved0:1;
	UINT32 BfAgcLnaGainx:2;
	UINT32 HtStfDet:1;
} RX_VECTOR2_3TH_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR2_3TH_CYCLE {
	UINT32 HtStfDet:1;
	UINT32 BfAgcLnaGainx:2;
	UINT32 Reserved0:1;
	UINT32 BfAgcLpfGain:4;
	UINT32 BfAgcIbRssix:8;
	UINT32 BgAgcWbRssix:8;
	UINT32 Reserved1:1;
	UINT32 OfdmCeRmsdId:3;
	UINT32 OfdmCeGicEnb:1;
	UINT32 OfdmCeLtfComb:1;
	UINT32 OfdmDewModeDet:1;
	UINT32 FcsErr:1;
} RX_VECTOR2_3TH_CYCLE;
#endif


typedef struct _RXV_INFO {
	INT32 Rssi0;
	INT32 Rssi1;
} RXV_INFO;

/* +++UNIFIED_TXD_RXD */
typedef struct GNU_PACKED _RXV_DWORD0 {
	UINT32 TA_0_31;
} RXV_DWORD0;
/* ---UNIFIED_TXD_RXD */


/* +++UNIFIED_TXD_RXD */
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RXV_DWORD1 {
	UINT32 Reserved:7;
	UINT32 TR:1;
	UINT32 RxvSn:8;
	UINT32 TA_32_47:16;
} RXV_DWORD1;
#else
typedef struct GNU_PACKED _RXV_DWORD1 {
	UINT32 TA_32_47:16;
	UINT32 RxvSn:8;
	UINT32 TR:1;
	UINT32 Reserved:7;
} RXV_DWORD1;
#endif
/* ---UNIFIED_TXD_RXD */


/* +++UNIFIED_TXD_RXD */
/* RX/TX-Status Vector Format */
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RXV_STRUC {
	UINT32 Ta0;
	UINT32 Rsv25To31:7;
	UINT32 Tr:1;
	UINT32 RxVSn:8;
	UINT32 Ta32:16;
	UINT32 Vector1[6];
	UINT32 Vector2[3];
} RXV_STRUC, *PRXV_STRUC;
#else
typedef struct GNU_PACKED _RXV_STRUC {
	UINT32 Ta0;
	UINT32 Ta32:16;
	UINT32 RxVSn:8;
	UINT32 Tr:1;
	UINT32 Rsv25To31:7;
	UINT32 Vector1[6];
	UINT32 Vector2[3];
} RXV_STRUC, *PRXV_STRUC;
#endif
/* ---UNIFIED_TXD_RXD */


/* +++UNIFIED_TXD_RXD */
typedef struct GNU_PACKED _RXV_FRM_STRUC {
	struct _RMAC_RXD_0_TXRXV RxVD0;
	RXV_STRUC RxVInfo[0];
} RXV_FRM_STRUC, *PRXV_FRM_STRUC;
/* ---UNIFIED_TXD_RXD */


/******************************************************************************
	LMAC TMR Structure

	TMR_D_1
	TMR_D_2
	TMR_D_6
******************************************************************************/

/******************************************************************************
	TMR_D_1
******************************************************************************/
/* +++UNIFIED_TXD_RXD */
/*
	Timing Measurement Report Format
*/
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _TMR_D_1 {
	struct {
		UINT32 TxvTxMode:3;
		UINT32 RxvDw1Bit29:1;
		UINT32 RxvDw1Bit23:1;
		UINT32 RxvTxMode:3;
		UINT32 RxvFrMode:2;
		UINT32 RxvDw1Bit12:1;
		UINT32 RxvDw3Bit6:1;
		UINT32 ToAeReport:6;
		UINT32 TodFine:6;
		UINT32 Pid:8;
	} field_init;

	struct {
		UINT32 TxvTxMode:3;
		UINT32 RxvDw1Bit29:1;
		UINT32 RxvDw1Bit23:1;
		UINT32 RxvTxMode:3;
		UINT32 RxvFrMode:2;
		UINT32 RxvDw1Bit12:1;
		UINT32 RxvDw3Bit6:1;
		UINT32 ToAeReport:6;
		UINT32 TodFine:6;
		UINT32 RxvSn:8;
	} field_resp;

	UINT32 word;
} TMR_D_1, *PTMR_D_1;
#else
typedef union GNU_PACKED _TMR_D_1 {
	struct {
		UINT32 Pid:8;
		UINT32 TodFine:6;
		UINT32 ToAeReport:6;
		UINT32 RxvDw3Bit6:1;
		UINT32 RxvDw1Bit12:1;
		UINT32 RxvFrMode:2;
		UINT32 RxvTxMode:3;
		UINT32 RxvDw1Bit23:1;
		UINT32 RxvDw1Bit29:1;
		UINT32 TxvTxMode:3;
	} field_init;

	struct {
		UINT32 RxvSn:8;
		UINT32 TodFine:6;
		UINT32 ToAeReport:6;
		UINT32 RxvDw3Bit6:1;
		UINT32 RxvDw1Bit12:1;
		UINT32 RxvFrMode:2;
		UINT32 RxvTxMode:3;
		UINT32 RxvDw1Bit23:1;
		UINT32 RxvDw1Bit29:1;
		UINT32 TxvTxMode:3;
	} field_resp;

	UINT32 word;
} TMR_D_1, *PTMR_D_1;
#endif
/* ---UNIFIED_TXD_RXD */


/******************************************************************************
	TMR_D_2
******************************************************************************/
/* +++UNIFIED_TXD_RXD */
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _TMR_D_2 {
	struct {
		UINT32 SnField:16;
		UINT32 Ta0:16;
	} field;

	UINT32 word;
} TMR_D_2, *PTMR_D_2;
#else
typedef union GNU_PACKED _TMR_D_2 {
	struct {
		UINT32 Ta0:16;
		UINT32 SnField:16;
	} field;

	UINT32 word;
} TMR_D_2, *PTMR_D_2;
#endif
/* ---UNIFIED_TXD_RXD */


/******************************************************************************
	TMR_D_6
******************************************************************************/
/* +++UNIFIED_TXD_RXD */
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _TMR_D_6 {
	struct {
		UINT32 ToA32:16;
		UINT32 ToD32:16;
	} field;

	UINT32 word;
} TMR_D_6, *PTMR_D_6;
#else
typedef union GNU_PACKED _TMR_D_6 {
	struct {
		UINT32 ToD32:16;
		UINT32 ToA32:16;
	} field;

	UINT32 word;
} TMR_D_6, *PTMR_D_6;
#endif
/* ---UNIFIED_TXD_RXD */


#ifdef TMR_GEN2
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _TMR_D_7 {
	struct {
		UINT32 Rsv:31;
		UINT32 DacOutput:1;
	} field;

	UINT32 word;
} TMR_D_7, *PTMR_D_7;
#else
typedef union GNU_PACKED _TMR_D_7 {
	struct {
		UINT32 DacOutput:1;
		UINT32 Rsv:31;
	} field;

	UINT32 word;
} TMR_D_7, *PTMR_D_7;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _TMR_D_8 {
	struct {
		UINT32 DBDC:1;
		UINT32 UnknownBit:1;
		UINT32 Rsv2:7;
		UINT32 RmsId:3;
		UINT32 FailReason:4;
		UINT32 Rsv1:5;
		UINT32 LtfSyncStartAddr:11;
	} field_ofdm;

	struct {
		UINT32 Rsv:29;
		UINT32 FailReason:3;
	} field_cck;

	UINT32 word;
} TMR_D_8, *PTMR_D_8;
#else
typedef union GNU_PACKED _TMR_D_8 {
	struct {
		UINT32 FailReason:3;
		UINT32 Rsv:29;
	} field_cck;

	struct {
		UINT32 LtfSyncStartAddr:11;
		UINT32 Rsv1:5;
		UINT32 FailReason:4;
		UINT32 RmsId:3;
		UINT32 Rsv2:7;
		UINT32 UnknownBit:1;
		UINT32 DBDC:1;
	} field_ofdm;

	UINT32 word;
} TMR_D_8, *PTMR_D_8;
#endif /* RT_BIG_ENDIAN */
#endif /* TMR_GEN2 */

/******************************************************************************
	TMR_FRM_STRUC
******************************************************************************/
/* +++UNIFIED_TXD_RXD */
typedef struct GNU_PACKED _TMR_FRM_STRUC {
	struct _RMAC_RXD_0_TMR TmrD0;
	TMR_D_1 TmrD1;
	TMR_D_2 TmrD2;
	UINT32 Ta16;
	UINT32 ToD0;
	UINT32 ToA0;
	TMR_D_6 TmrD6;
#ifdef TMR_GEN2
	TMR_D_7 TmrD7;
	TMR_D_8 TmrD8;
#endif
} TMR_FRM_STRUC, *PTMR_FRM_STRUC;
/* ---UNIFIED_TXD_RXD */


typedef struct wtbl_entry {
	UINT8 wtbl_idx;
	struct wtbl_struc wtbl;
	UINT32 wtbl_addr;
} WTBL_ENTRY;





/* TODO: shiang-MT7615 */
#define MT7615_MT_WTBL_SIZE	128
#define MT7622_MT_WTBL_SIZE	128
#define MT_DMAC_BA_AGG_RANGE		8
#define MT_DMAC_BA_STAT_RANGE	8

#define MT_PSE_BASE_ADDR		0xa0000000
#define MT_PSE_PAGE_SIZE		128

#define MT_PCI_REMAP_ADDR_1	0x40000
#define MT_PCI_REMAP_ADDR_2	0x80000

#ifdef MAC_REPEATER_SUPPORT
#define MAX_EXT_MAC_ADDR_SIZE	32
/* TODO: Carter, single driver case? */
#endif /* MAC_REPEATER_SUPPORT */


/* TODO: shiang-MT7615, move to top.h! */
#define MCU_CFG_BASE		0x2000
#define MCU_COM_REG1	    (MCU_CFG_BASE + 0x204)
#ifdef ERR_RECOVERY
#define MCU_COM_REG1_SER_PSE		BIT(0)
#define MCU_COM_REG1_SER_PLE		BIT(1)
#define MCU_COM_REG1_SER_PCIE		BIT(2)
#define MCU_COM_REG1_SER_PDMA		BIT(3)
#define MCU_COM_REG1_SER_LMAC_TX	BIT(4)
#define MCU_COM_REG1_SER_SEC_RF_RX	BIT(5)
#endif  /* ERR_RECOVERY */

#define MCU_PCIE_REMAP_1	(MCU_CFG_BASE + 0x500)
#define REMAP_1_OFFSET_MASK (0x3ffff)
#define GET_REMAP_1_OFFSET(p) (((p) & REMAP_1_OFFSET_MASK))
#define REMAP_1_BASE_MASK	(0x3fff << 18)
#define GET_REMAP_1_BASE(p) (((p) & REMAP_1_BASE_MASK) >> 18)
#define MCU_PCIE_REMAP_2	(MCU_CFG_BASE + 0x504)
#define REMAP_2_OFFSET_MASK (0x7ffff)
#define GET_REMAP_2_OFFSET(p) (((p) & REMAP_2_OFFSET_MASK))
#define REMAP_2_BASE_MASK (0x1fff << 19)
#define GET_REMAP_2_BASE(p) (((p) & REMAP_2_BASE_MASK) >> 19)

/* leo: defined in top.h #define TOP_CFG_BASE        0x0000 */

#define XTAL_CTL4           (TOP_CFG_BASE + 0x1210)
#define XTAL_CTL13          (TOP_CFG_BASE + 0x1234)
#define XTAL_CTL14          (TOP_CFG_BASE + 0x1238)
#define DA_XO_C2_MASK (0x7f << 8)
#define DA_XO_C2(p) (((p) & 0x7f) << 8)


/* Indicate the txs report to Host or MCU */
typedef enum {
	TXS2MCU = 0,
	TXS2HOST = 1,
} TXS_TARGET_PORT;

#define TXS2MCU_AGGNUMS 31
#define TXS2HOST_AGGNUMS 31

enum {
	TXS2M_QID0,
	TXS2M_QID1,
	TXS2M_QID2,
	TXS2M_QID3,
};

enum {
	TXS2H_QID0,
	TXS2H_QID1,
};

enum {
	TXS_FORMAT0,
	TXS_FORMAT1,
};

#define SHAREDKEYTABLE			0
#define PAIRWISEKEYTABLE		1


#define TSO_SIZE		0

#define EXP_ACK_TIME	0x1380

/* [0:7]=RX_MAX_PTIME (unit=20us), [8:14]=RX_MAX_PINT, [15]=RX_DLY_INT_EN */
#define RX_DLY_INT_CFG  (0x811c)

#ifdef RT_BIG_ENDIAN
typedef	union GNU_PACKED _TX_STA_CNT0_STRUC {
	struct {
		UINT16  TxBeaconCount;
		UINT16  TxFailCount;
	} field;
	UINT32 word;
} TX_STA_CNT0_STRUC;
#else
typedef	union GNU_PACKED _TX_STA_CNT0_STRUC {
	struct {
		UINT16  TxFailCount;
		UINT16  TxBeaconCount;
	} field;
	UINT32 word;
} TX_STA_CNT0_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef	union GNU_PACKED _TX_STA_CNT1_STRUC {
	struct {
		UINT16  TxRetransmit;
		UINT16  TxSuccess;
	} field;
	UINT32 word;
} TX_STA_CNT1_STRUC;
#else
typedef	union GNU_PACKED _TX_STA_CNT1_STRUC {
	struct {
		UINT16  TxSuccess;
		UINT16  TxRetransmit;
	} field;
	UINT32 word;
} TX_STA_CNT1_STRUC;
#endif

typedef struct __DMAC_TX_CNT_INFO {
	union WTBL_DW14 wtbl_wd14;
	union WTBL_DW15 wtbl_wd15;
	union WTBL_DW16 wtbl_wd16;
	union WTBL_DW17 wtbl_wd17;
} DMAC_TX_CNT_INFO;


#define E2PROM_CSR          0x0004
#define GPIO_CTRL_CFG	0x0228
#define WSC_HDR_BTN_GPIO_0			((UINT32)0x00000001) /* bit 0 for RT2860/RT2870 */
#define WSC_HDR_BTN_GPIO_3			((UINT32)0x00000008) /* bit 3 for RT2860/RT2870 */

#ifdef DBDC_MODE
#define DBDC_BAND_NUM		2
#else
#define DBDC_BAND_NUM		1
#endif /* DBDC_MODE */

#define DBDC_BAND0		0
#define DBDC_BAND1		1

struct _RTMP_ADAPTER;

VOID dump_mt_mac_cr(struct _RTMP_ADAPTER *pAd);
INT dump_dmac_amsdu_info(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID Update_Mib_Bucket_One_Sec(struct _RTMP_ADAPTER *pAd);
VOID Update_Mib_Bucket_500Ms(struct _RTMP_ADAPTER *pAd);

INT mt_wtbl_init_ByFw(struct _RTMP_ADAPTER *pAd);
INT mt_wtbl_init_ByDriver(struct _RTMP_ADAPTER *pAd);
INT mt_mac_init(struct _RTMP_ADAPTER *pAd);
INT mt_hw_tb_init(struct _RTMP_ADAPTER *pAd, BOOLEAN bHardReset);

INT mt_wtbl_get_entry234(struct _RTMP_ADAPTER *pAd, UCHAR idx, struct wtbl_entry *ent);
VOID dump_wtbl_entry(struct _RTMP_ADAPTER *pAd, struct wtbl_entry *ent);
VOID dump_wtbl_info(struct _RTMP_ADAPTER *pAd, UINT wtbl_idx);
VOID dump_wtbl_base_info(struct _RTMP_ADAPTER *pAd);

INT mt_mac_set_ctrlch(struct _RTMP_ADAPTER *pAd, UINT8 extch);
#ifdef GREENAP_SUPPORT
INT rtmp_mac_set_mmps(struct _RTMP_ADAPTER *pAd, INT ReduceCorePower);
#endif /* GREENAP_SUPPORT */

UINT16 tx_rate_to_tmi_rate(UINT8 mode, UINT8 mcs, UINT8 nss, BOOLEAN stbc, UINT8 preamble);
UCHAR get_nsts_by_mcs(UCHAR phy_mode, UCHAR mcs, BOOLEAN stbc, UCHAR vht_nss);
VOID mt_chip_info_show(struct _RTMP_ADAPTER *pAd);

#define HIF_PORT 1
#define MCU_PORT 2
#define MT_TX_RETRY_UNLIMIT		0x1f
#define MT_TX_SHORT_RETRY		0x07
#define MT_TX_LONG_RETRY		0x0f

INT mt_nic_asic_init(struct _RTMP_ADAPTER *pAd);

struct _CR4_TXP_MSDU_INFO;
struct _TX_BLK;
struct _MAC_TX_INFO;
union _HTTRANSMIT_SETTING;
struct _TMAC_INFO;
INT32 dump_txp_info(struct _RTMP_ADAPTER *pAd, struct _CR4_TXP_MSDU_INFO *txp_info);
INT32 mtd_write_txp_info_by_cr4(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
INT32 mtd_write_txp_info_by_host(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
INT32 mtd_write_txp_info_by_host_v2(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
VOID mtd_write_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
VOID mtd_write_tmac_info_by_cr4(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
VOID mtd_write_tmac_info_by_host(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
INT mtd_check_hw_resource(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR resource_idx);
INT mtd_hw_tx(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk);
INT mtd_mlme_hw_tx(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info, struct _MAC_TX_INFO *info, union _HTTRANSMIT_SETTING *transmit, struct _TX_BLK *tx_blk);
INT32 mtd_ate_hw_tx(struct _RTMP_ADAPTER *pAd, struct _TMAC_INFO *info, struct _TX_BLK *tx_blk);
BOOLEAN in_altx_filter_list(UCHAR sub_type);

extern UCHAR dmac_wmm_swq_2_hw_ac_que[4][4];
extern const UCHAR altx_filter_list[];

#endif /* __MT_DMAC_H__ */

