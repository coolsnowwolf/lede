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
	mt_smac.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __MT_SMAC_H__
#define __MT_SMAC_H__

#include "mac/mac_mt/top.h"
#include "mac/mac_mt/smac/gpio.h"
#include "mac/mac_mt/smac/wf_agg.h"
#include "mac/mac_mt/smac/wf_aon.h"
#include "mac/mac_mt/smac/wf_arb.h"
#include "mac/mac_mt/smac/wf_cfgoff.h"
#include "mac/mac_mt/smac/wf_dma.h"
#include "mac/mac_mt/smac/wf_lpon_top.h"
#include "mac/mac_mt/smac/wf_mib.h"
#include "mac/mac_mt/smac/wf_pf.h"
#include "mac/mac_mt/smac/wf_rmac.h"
#include "mac/mac_mt/smac/wf_sec.h"
#include "mac/mac_mt/smac/wf_tmac.h"
#include "mac/mac_mt/smac/wf_trb.h"
#include "mac/mac_mt/smac/wf_wtbloff.h"
#include "mac/mac_mt/smac/wf_wtblon.h"
#include "mac/mac_mt/smac/wf_int_wakeup_top.h"
#include "mac/mac_mt/smac/client.h"
#include "mac/mac_mt/smac/wf_wtbl.h"
#include "mac/mac_mt/smac/wf_phy.h"
#include "mac/mac_mt/smac/pse.h"

#ifdef DMA_SCH_SUPPORT
#include "mac/mac_mt/smac/dma_sch.h"
#endif /* DMA_SCH_SUPPORT */

#ifndef COMPOS_WIN
#endif /* COMPOS_WIN */



/*
	TX / RX descriptor format

	TX:
		PCI/RBUS_DMA_Descriptor + TXD + 802.11

	Rx:
		PCI/RBUS/USB_DMA_Descripotr + RXD + 802.11 + RX_CSO_INFO
*/

#define FIFO_MGMT	0
#define FIFO_HCCA	1
#define FIFO_EDCA	2


#ifdef MAC_REPEATER_SUPPORT
#define MAX_EXT_MAC_ADDR_SIZE	16
/* TODO: Carter, single driver case? */
#endif /* MAC_REPEATER_SUPPORT */


/******************************************************************************
	LMAC TxD

	TMAC_TXD_0
	TMAC_TXD_1
	TMAC_TXD_2
	TMAC_TXD_3

******************************************************************************/

/******************************************************************************
	TMAC_TXD_0
******************************************************************************/

/* +++UNIFIED_TXD_RXD */
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TMAC_TXD_0 {
	/*  DWORD 0 */
	UINT32 PIdx:1;
	UINT32 QIdx:4;
	UINT32 UTxB:1;
	UINT32 UNxV:1;
	UINT32 UTChkSum:1;
	UINT32 IpChkSum:1;
	UINT32 EthTypeOffset:7;
	UINT32 TxByteCount:16;
} TMAC_TXD_0, *PTMAC_TXD_0;
#else
typedef struct GNU_PACKED _TMAC_TXD_0 {
	/* DWORD 0 */
	UINT32 TxByteCount:16;
	UINT32 EthTypeOffset:7;
	UINT32 IpChkSum:1;
	UINT32 UTChkSum:1;
	UINT32 UNxV:1;
	UINT32 UTxB:1;
	UINT32 QIdx:4;
	UINT32 PIdx:1;
} TMAC_TXD_0, *PTMAC_TXD_0;
#endif
/* ---UNIFIED_TXD_RXD */

/* TMAC_TXD_0.PIdx */
#define P_IDX_LMAC	0
#define P_IDX_MCU	1

/* TMAC_TXD_0.QIdx */
#define Q_IDX_AC0	0x00
#define Q_IDX_AC1	0x01
#define Q_IDX_AC2	0x02
#define Q_IDX_AC3	0x03
#define Q_IDX_AC4	0x04
#define Q_IDX_AC5	0x05
#define Q_IDX_AC6	0x06
#define Q_IDX_BCN	0x07
#define TxQ_IDX_BCN0 Q_IDX_BCN
#define Q_IDX_BMC	0x08
#define Q_IDX_AC10	0x09
#define Q_IDX_AC11	0x0a
#define Q_IDX_AC12	0x0b
#define Q_IDX_AC13	0x0c
#define Q_IDX_AC14	0x0d

#define Q_IDX_RQ0	0x00
#define Q_IDX_RQ1	0x01
#define Q_IDX_RQ2	0x02
#define Q_IDX_RQ3	0x03

/* TMAC_TXD_0.PIdx +  TMAC_TXD_0.QIdx */
#define PQ_IDX_LMAC_AC0	((P_IDX_LMAC << 4) | (Q_IDX_AC0))
#define PQ_IDX_LMAC_AC1	((P_IDX_LMAC << 4) | (Q_IDX_AC1))
#define PQ_IDX_LMAC_AC2	((P_IDX_LMAC << 4) | (Q_IDX_AC2))
#define PQ_IDX_LMAC_AC3	((P_IDX_LMAC << 4) | (Q_IDX_AC3))
#define PQ_IDX_LMAC_AC4	((P_IDX_LMAC << 4) | (Q_IDX_AC4))
#define PQ_IDX_LMAC_AC5	((P_IDX_LMAC << 4) | (Q_IDX_AC5))
#define PQ_IDX_LMAC_AC6	((P_IDX_LMAC << 4) | (Q_IDX_AC6))
#define PQ_IDX_LMAC_BCN	((P_IDX_LMAC << 4) | (Q_IDX_BCN))
#define PQ_IDX_LMAC_BMC	((P_IDX_LMAC << 4) | (Q_IDX_BMC))
#define PQ_IDX_LMAC_AC10	((P_IDX_LMAC << 4) | (Q_IDX_AC10))
#define PQ_IDX_LMAC_AC11	((P_IDX_LMAC << 4) | (Q_IDX_AC11))
#define PQ_IDX_LMAC_AC12	((P_IDX_LMAC << 4) | (Q_IDX_AC12))
#define PQ_IDX_LMAC_AC13	((P_IDX_LMAC << 4) | (Q_IDX_AC13))
#define PQ_IDX_LMAC_AC14	((P_IDX_LMAC << 4) | (Q_IDX_AC14))

#define PQ_IDX_MCU_RQ0		((P_IDX_MCU << 4) | (Q_IDX_RQ0))
#define PQ_IDX_MCU_RQ1		((P_IDX_MCU << 4) | (Q_IDX_RQ1))
#define PQ_IDX_MCU_RQ2		((P_IDX_MCU << 4) | (Q_IDX_RQ2))
#define PQ_IDX_MCU_RQ3		((P_IDX_MCU << 4) | (Q_IDX_RQ3))


/******************************************************************************
	TMAC_TXD_1
******************************************************************************/

/* +++UNIFIED_TXD_RXD */
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TMAC_TXD_1 {
	/* DWORD 1 */
	UINT32 OwnMacAddr:6;
	UINT32 Rsv24to25:2;
	UINT32 ProtectFrame:1;
	UINT32 Tid:3;
	UINT32 NoAck:1;
	UINT32 HdrPad:3;
	UINT32 TxDFmt:1;
	UINT32 HdrFmt:2;
	UINT32 HdrInfo:5;
	UINT32 WlanIdx:8;
} TMAC_TXD_1, *PTMAC_TXD_1;
#else
typedef struct GNU_PACKED _TMAC_TXD_1 {
	/* DWORD 1 */
	UINT32 WlanIdx:8;
	UINT32 HdrInfo:5;
	UINT32 HdrFmt:2;
	UINT32 TxDFmt:1;
	UINT32 HdrPad:3;
	UINT32 NoAck:1;
	UINT32 Tid:3;
	UINT32 ProtectFrame:1;
	UINT32 Rsv24to25:2;
	UINT32 OwnMacAddr:6;
} TMAC_TXD_1, *PTMAC_TXD_1;
#endif
/* ---UNIFIED_TXD_RXD */

/* TMAC_TXD_1.HdrFmt */
#define TMI_HDR_FT_NON_80211	0x0
#define TMI_HDR_FT_CMD		0x1
#define TMI_HDR_FT_NOR_80211	0x2
#define TMI_HDR_FT_ENH_80211	0x3

/*
	TMAC_TXD_1.HdrInfo
*/
/* if TMAC_TXD_1.HdrFmt  == HDR_FORMAT_NON_80211 */
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


/* if TMAC_TXD_1.HdrFmt  == HDR_FORMAT_CMD */
#define TMI_HDR_INFO_1_MASK_RSV		(0x1f)
#define TMI_HDR_INFO_1_VAL			0

/* if TMAC_TXD_1.HdrFmt  == HDR_FORMAT_NOR_80211 */
#define TMI_HDR_INFO_2_MASK_LEN	(0x1f)
#define TMI_HDR_INFO_2_VAL(_len)	(_len >> 1)

/* if TMAC_TXD_1.HdrFmt  == HDR_FORMAT_ENH_80211 */
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
#define TMI_HDR_PAD_BIT_MODE		2
#define TMI_HDR_PAD_MODE_TAIL		0
#define TMI_HDR_PAD_MODE_HEAD	1
#define TMI_HDR_PAD_BIT_LEN	0
#define TMI_HDR_PAD_MASK_LEN	0x3


/******************************************************************************
	TMAC_TXD_2
******************************************************************************/


/* +++UNIFIED_TXD_RXD */
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TMAC_TXD_2 {
	/* DWORD 2 */
	UINT32 FixRate:1;
	UINT32 TimingMeasure:1;
	UINT32 BaDisable:1;
	UINT32 PwrOffset:5;
	UINT32 MaxTxTime:8;
	UINT32 Frag:2;
	UINT32 HtcExist:1;
	UINT32 Duration:1;
	UINT32 Bip:1;
	UINT32 BmcPkt:1;
	UINT32 Rts:1;
	UINT32 Sounding:1;
	UINT32 Ndpa:1;
	UINT32 Ndp:1;
	UINT32 FrmType:2;
	UINT32 SubType:4;
} TMAC_TXD_2, *PTMAC_TXD_2;
#else
typedef struct GNU_PACKED _TMAC_TXD_2 {
	/* DWORD 2 */
	UINT32 SubType:4;
	UINT32 FrmType:2;
	UINT32 Ndp:1;
	UINT32 Ndpa:1;
	UINT32 Sounding:1;
	UINT32 Rts:1;
	UINT32 BmcPkt:1;
	UINT32 Bip:1;
	UINT32 Duration:1;
	UINT32 HtcExist:1;
	UINT32 Frag:2;
	UINT32 MaxTxTime:8;
	UINT32 PwrOffset:5;
	UINT32 BaDisable:1;
	UINT32 TimingMeasure:1;
	UINT32 FixRate:1;
} TMAC_TXD_2, *PTMAC_TXD_2;
#endif
/* ---UNIFIED_TXD_RXD */

/******************************************************************************
	TMAC_TXD_3
******************************************************************************/

/* +++UNIFIED_TXD_RXD */
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TMAC_TXD_3 {
	/* DWORD 3 */
	UINT32 SnVld:1;
	UINT32 PnVld:1;
	UINT32 Rsv28to29:2;
	UINT32 Sn:12;
	UINT32 RemainTxCnt:5;
	UINT32 TxCnt:5;
	UINT32 Rsv0to5:6;
} TMAC_TXD_3, *PTMAC_TXD_3;
#else
typedef struct GNU_PACKED _TMAC_TXD_3 {
	/* DWORD 3 */
	UINT32 Rsv0to5:6;
	UINT32 TxCnt:5;
	UINT32 RemainTxCnt:5;
	UINT32 Sn:12;
	UINT32 Rsv28to29:2;
	UINT32 PnVld:1;
	UINT32 SnVld:1;
} TMAC_TXD_3, *PTMAC_TXD_3;
#endif
/* ---UNIFIED_TXD_RXD */

/******************************************************************************
	TMAC_TXD_4
******************************************************************************/
/* +++UNIFIED_TXD_RXD */
typedef struct GNU_PACKED _TMAC_TXD_4 {
	UINT32   PktNumLow;
} TMAC_TXD_4, *PTMAC_TXD_4;
/* ---UNIFIED_TXD_RXD */


/******************************************************************************
	TMAC_TXD_5
******************************************************************************/
/* +++UNIFIED_TXD_RXD */
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TMAC_TXD_5 {
	UINT32   PktNumHigh:16;
	UINT32   Rsv:2;
	UINT32   PwrMgmt:1;
	UINT32   BarSsnCtrl:1;
	UINT32   DataSrcSelect:1;
	UINT32   TxS2Host:1;
	UINT32   TxS2Mcu:1;
	UINT32   TxSFmt:1;
	UINT32   PktId:8;
} TMAC_TXD_5, *PTMAC_TXD_5;
#else
typedef struct GNU_PACKED _TMAC_TXD_5 {
	UINT32   PktId:8;
	UINT32   TxSFmt:1;
	UINT32   TxS2Mcu:1;
	UINT32   TxS2Host:1;
	UINT32   DataSrcSelect:1;
	UINT32   BarSsnCtrl:1;
	UINT32   PwrMgmt:1;
	UINT32   Rsv:2;
	UINT32   PktNumHigh:16;
} TMAC_TXD_5, *PTMAC_TXD_5;
#endif
/* ---UNIFIED_TXD_RXD */

/* TMAC_TXD_5.DataSrcSelect */
#define TMI_DAS_FROM_MPDU		0
#define TMI_DAS_FROM_WTBL		1

/* TMAC_TXD_5.BarSsnCtrl */
#define TMI_BSN_CFG_BY_HW	0
#define TMI_BSN_CFG_BY_SW	1

/* TMAC_TXD_5.PwrMgmt */
#define TMI_PM_BIT_CFG_BY_HW	0
#define TMI_PM_BIT_CFG_BY_SW	1


/******************************************************************************
	TMAC_TXD_6
******************************************************************************/
/* +++UNIFIED_TXD_RXD */

#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TMAC_TXD_6 {
	UINT32   GI:1;
	UINT32   LDPC:1;
	UINT32   RateToBeFixed:12;
	UINT32   ImplicitBf:1;
	UINT32   ExplicitBf:1;
	UINT32   DynamicBw:1;
	UINT32   AntPri:3;
	UINT32   SpeExtEnable:1;
	UINT32   FixedBwMode:3;
	UINT32   AntIdx:6;
	UINT32   Rsv:1;
	UINT32   FixedRateMode:1;
} TMAC_TXD_6, *PTMAC_TXD_6;
#else
typedef struct GNU_PACKED _TMAC_TXD_6 {
	UINT32   FixedRateMode:1;
	UINT32   Rsv:1;
	UINT32   AntIdx:6;
	UINT32   FixedBwMode:3;
	UINT32   SpeExtEnable:1;
	UINT32   AntPri:3;
	UINT32   DynamicBw:1;
	UINT32   ExplicitBf:1;
	UINT32   ImplicitBf:1;
	UINT32   RateToBeFixed:12;
	UINT32   LDPC:1;
	UINT32   GI:1;
} TMAC_TXD_6, *PTMAC_TXD_6;
#endif
/* ---UNIFIED_TXD_RXD */


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

/* +++UNIFIED_TXD_RXD */
#ifdef RT_BIG_ENDIAN
/* TODO: Lens, need to make more specific and clear definition for those fields! */
typedef struct _TMAC_TXD_7 {
	UINT32   SwUseNonQoS:1; /* remove */
	UINT32   SwUseSegmentEnd:1;
	UINT32   SwUseAMSDU:1; /* remove */
	UINT32   SwUseUSB4ByteAlign:2; /* remove */
	UINT32   SwUseSegIdx:6;
	UINT32   SwFieldReserve:5;
	UINT32   SchdTxTime:16;
} TMAC_TXD_7, *PTMAC_TXD_7;
#else
typedef struct _TMAC_TXD_7 {
	UINT32   SchdTxTime:16;
	UINT32   SwFieldReserve:5;
	UINT32   SwUseSegIdx:6;
	UINT32   SwUseUSB4ByteAlign:2;
	UINT32   SwUseAMSDU:1;
	UINT32   SwUseSegmentEnd:1;
	UINT32   SwUseNonQoS:1;
} TMAC_TXD_7, *PTMAC_TXD_7;
#endif
/* ---UNIFIED_TXD_RXD */


/******************************************************************************
	TX Descriptor in Long Format (TMAC_TXD_1.FT = 1)
	which including TMAC_TXD_0~ TMAC_TXD_7
******************************************************************************/
/* +++UNIFIED_TXD_RXD */
typedef struct GNU_PACKED _TMAC_TXD_L {
	TMAC_TXD_0 TxD0;
	TMAC_TXD_1 TxD1;
	TMAC_TXD_2 TxD2;
	TMAC_TXD_3 TxD3;
	TMAC_TXD_4 TxD4;
	TMAC_TXD_5 TxD5;
	TMAC_TXD_6 TxD6;
	TMAC_TXD_7 TxD7;
} TMAC_TXD_L, *PTMAC_TXD_L;
/* ---UNIFIED_TXD_RXD */


/******************************************************************************
	TX Descriptor in Short Format (TMAC_TXD_1.FT = 0)
	which including TMAC_TXD_0, TMAC_TXD_1, and TMAC_TXD_7
******************************************************************************/
/*
	TX Descriptor in Short Format (TMAC_TXD_1.FT = 0)
*/
/* +++UNIFIED_TXD_RXD */
typedef struct GNU_PACKED _TMAC_TXD_S {
	TMAC_TXD_0 TxD0;
	TMAC_TXD_1 TxD1;
	TMAC_TXD_7 TxD7;
} TMAC_TXD_S, *PTMAC_TXD_S;
/* ---UNIFIED_TXD_RXD */


typedef struct _TXV1_MAC2PHY {
	UCHAR txrate;
	UCHAR STBC;
	UCHAR Ldpc_Bcc;
	UCHAR Ness;
	UCHAR txmode;
	UCHAR bw;
	UCHAR txpower;
} TXV1_MAC2PHY;

typedef struct _TXV2_MAC2PHY {
	UCHAR agg;
	UCHAR gi;
	UCHAR smooth;
	UCHAR sounding;
	UCHAR txbf;
} TXV2_MAC2PHY;

typedef struct _TXV3_MAC2PHY {
	UCHAR bw_status;
	UCHAR mac[6];
} TXV3_MAC2PHY;



#define RMAC_RXD_0_PKT_TYPE_MASK		0xe0000000
#define RMAC_RX_PKT_TYPE_RX_TXS			0x00
#define RMAC_RX_PKT_TYPE_RX_TXRXV		0x01
#define RMAC_RX_PKT_TYPE_RX_NORMAL		0x02
#define RMAC_RX_PKT_TYPE_RX_DUP_RFB		0x03
#define RMAC_RX_PKT_TYPE_RX_TMR			0x04
#define RMAC_RX_PKT_TYPE_RETRIEVE		0x05
#define RMAC_RX_PKT_TYPE_RX_EVENT		0x07
#define RMAC_RX_PKT_TYPE(_x)			(((_x) & RMAC_RXD_0_PKT_TYPE_MASK) >> 29)
#define RMAC_RXD_0_PKT_RX_BYTE_COUNT_MASK 0x0000ffff
#define RMAC_RX_PKT_RX_BYTE_COUNT(_x)	(((_x) & RMAC_RXD_0_PKT_RX_BYTE_COUNT_MASK))

/* +++UNIFIED_TXD_RXD */
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
/* ---UNIFIED_TXD_RXD */


/* +++UNIFIED_TXD_RXD */
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
/* ---UNIFIED_TXD_RXD */


/******************************************************************************
	LMAC RxD

	RMAC_RXD_0
	RMAC_RXD_1
	RMAC_RXD_2
	RMAC_RXD_3

******************************************************************************/

/******************************************************************************
	RMAC_RXD_0
******************************************************************************/
/* +++UNIFIED_TXD_RXD */
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RMAC_RXD_0 {
	UINT32   PktType:3;
	UINT32   RfbGroupVld:4;
	UINT32   UdpTcpChkSumOffload:1;
	UINT32   IpChkSumOffload:1;
	UINT32   EthTypeOffset:7;
	UINT32   RxByteCnt:16;
} RMAC_RXD_0, *PRMAC_RXD_0;
#else
typedef struct GNU_PACKED _RMAC_RXD_0 {
	UINT32   RxByteCnt:16;
	UINT32   EthTypeOffset:7;
	UINT32   IpChkSumOffload:1;
	UINT32   UdpTcpChkSumOffload:1;
	UINT32   RfbGroupVld:4;
	UINT32   PktType:3;
} RMAC_RXD_0, *PRMAC_RXD_0;
#endif
/* ---UNIFIED_TXD_RXD */

/******************************************************************************
	RMAC_RXD_1
******************************************************************************/
/* +++UNIFIED_TXD_RXD */
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
	UINT32   Bcast:1;
	UINT32   Mcast:1;
	UINT32   UcastToMe:1;
	UINT32   HTC:1;
} RMAC_RXD_1, *PRMAC_RXD_1;
#else
typedef struct GNU_PACKED _RMAC_RXD_1 {
	UINT32   HTC:1;
	UINT32   UcastToMe:1;
	UINT32   Mcast:1;
	UINT32   Bcast:1;
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
/* ---UNIFIED_TXD_RXD */

/******************************************************************************
	RMAC_RXD_2
******************************************************************************/
/* +++UNIFIED_TXD_RXD */
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RMAC_RXD_2 {
	UINT32   NonAmpduFrm:1;
	UINT32   NonAmpduSfrm:1;
	UINT32   NonDataFrm:1;
	UINT32   NullFrm:1;
	UINT32   FragFrm:1;
	UINT32   UdfVlanType:1;
	UINT32   LlcMis:1;
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
	UINT32   LlcMis:1;
	UINT32   UdfVlanType:1;
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
	UINT32 Type:2;
	UINT32 SubType:4;
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
	UINT32 SubType:4;
	UINT32 Type:2;
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
******************************************************************************/

/******************************************************************************
	TXS_D_0
******************************************************************************/
/* +++UNIFIED_TXD_RXD */
/* TX Status Report Format */
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXS_D_0 {
	UINT32 AntId:6;
	UINT32 Tid:3;
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
	UINT32 Tid:3;
	UINT32 AntId:6;
} TXS_D_0, *PTXS_D_0;
#endif
/* ---UNIFIED_TXD_RXD */


/******************************************************************************
	TXS_D_1
******************************************************************************/
/* +++UNIFIED_TXD_RXD */
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _TXS_D_1 {
	struct {
		UINT32 Rsv:8;
		UINT32 Noise2:8;
		UINT32 Noise1:8;
		UINT32 Noise0:8;
	} FieldNoise;

	UINT32 TimeStamp;

	UINT32 Word;
} TXS_D_1, *PTXS_D_1;
#else
typedef union GNU_PACKED _TXS_D_1 {
	struct {
		UINT32 Noise0:8;
		UINT32 Noise1:8;
		UINT32 Noise2:8;
		UINT32 Rsv:8;
	} FieldNoise;

	UINT32 TimeStamp;

	UINT32 Word;
} TXS_D_1, *PTXS_D_1;
#endif
/* ---UNIFIED_TXD_RXD */


/******************************************************************************
	TXS_D_2
******************************************************************************/
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _TXS_D_2 {
	struct {
		UINT32 TxPwrdBm:7;
		UINT32 FrontTime:25;
	} field_ft;

	struct {
		UINT32 TxPwrdBm:7;
		UINT32 Rsv:1;
		UINT32 RCPI2:8;
		UINT32 RCPI1:8;
		UINT32 RCPI0:8;
	} field_rcpi;

	UINT32 Word;
} TXS_D_2, *PTXS_D_2;
#else
typedef union GNU_PACKED _TXS_D_2 {
	struct {
		UINT32 FrontTime:25;
		UINT32 TxPwrdBm:7;
	} field_ft;

	struct {
		UINT32 RCPI0:8;
		UINT32 RCPI1:8;
		UINT32 RCPI2:8;
		UINT32 Rsv:1;
		UINT32 TxPwrdBm:7;
	} field_rcpi;

	UINT32 Word;
} TXS_D_2, *PTXS_D_2;
#endif


/******************************************************************************
	TXS_D_3
******************************************************************************/
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXS_D_3 {
	UINT32 TxS_WlanIdx:8;
	UINT32 RXV_SN:8;
	UINT32 TxDelay:16;
} TXS_D_3, *PTXS_D_3;
#else
typedef struct GNU_PACKED _TXS_D_3 {
	UINT32 TxDelay:16;
	UINT32 RXV_SN:8;
	UINT32 TxS_WlanIdx:8;
} TXS_D_3, *PTXS_D_3;
#endif


/******************************************************************************
	TXS_D_4
******************************************************************************/
#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXS_D_4 {
	UINT32 TxS_LastTxMcsIdx:3;
	UINT32 TxS_MpduTxCnt:5;
	UINT32 TxS_AMPDU:1;
	UINT32 TxS_FianlMPDU:1;
	UINT32 TxS_Pid:8;
	UINT32 TxS_TxBW:2;
	UINT32 TxS_SN_TSSI:12;
} TXS_D_4, *PTXS_D_4;
#else
typedef struct GNU_PACKED _TXS_D_4 {
	UINT32 TxS_SN_TSSI:12;
	UINT32 TxS_TxBW:2;
	UINT32 TxS_Pid:8;
	UINT32 TxS_FianlMPDU:1;
	UINT32 TxS_AMPDU:1;
	UINT32 TxS_MpduTxCnt:5;
	UINT32 TxS_LastTxMcsIdx:3;
} TXS_D_4, *PTXS_D_4;
#endif


/******************************************************************************
	TXS_STRUC
******************************************************************************/
typedef struct GNU_PACKED _TXS_STRUC {
	TXS_D_0 TxSD0;
	TXS_D_1 TxSD1;
	TXS_D_2 TxSD2;
	TXS_D_3 TxSD3;
	TXS_D_4 TxSD4;
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
	UINT32 VHTA1_B5_B4:2;
	UINT32 VHTA2_B8_B1:8;
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
	UINT32 HtAdCode:1;
	UINT32 HtExtltf:2;
	UINT32 TxMode:3;
	UINT32 FrMode:2;
	UINT32 VHTA1_B22:1;
	UINT32 HtAggregation:1;
	UINT32 HtShortGi:1;
	UINT32 HtSmooth:1;
	UINT32 HtNoSound:1;
	UINT32 VHTA2_B8_B1:8;
	UINT32 VHTA1_B5_B4:2;
} RX_VECTOR1_1ST_CYCLE;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR1_2ND_CYCLE {
	UINT32 VHTA1_B16_B6:11;
	UINT32 Length:21;
} RX_VECTOR1_2ND_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR1_2ND_CYCLE {
	UINT32 Length:21;
	UINT32 VHTA1_B16_B6:11;
} RX_VECTOR1_2ND_CYCLE;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR1_3TH_CYCLE {
	UINT32 Fagc1CalGain:3;
	UINT32 Fgac1EqCal:1;
	UINT32 Rcpi1:8;
	UINT32 Fagc0CalGain:3;
	UINT32 Fagc0EqCal:1;
	UINT32 Rcpi0:8;
	UINT32 SelAnt:1;
	UINT32 ACI_DETx:1;
	UINT32 OFDMFreqTransDet:1;
	UINT32 VHTA1_B21_B17:5;
} RX_VECTOR1_3TH_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR1_3TH_CYCLE {
	UINT32 VHTA1_B21_B17:5;
	UINT32 OFDMFreqTransDet:1;
	UINT32 ACI_DETx:1;
	UINT32 SelAnt:1;
	UINT32 Rcpi0:8;
	UINT32 Fagc0EqCal:1;
	UINT32 Fgac0CalGain:3;
	UINT32 Rcpi1:8;
	UINT32 Fagc1EqCal:1;
	UINT32 Fgac1CalGain:3;
} RX_VECTOR1_3TH_CYCLE;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR1_4TH_CYCLE {
	UINT32 FgacCalGain:3;
	UINT32 Fagc2EqCal:1;
	UINT32 IBRssi1:8;
	UINT32 FagcLpfGainx:4;
	UINT32 WBRssix:8;
	UINT32 IBRssi0:8;
} RX_VECTOR1_4TH_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR1_4TH_CYCLE {
	UINT32 IBRssi0:8;
	UINT32 WBRssix:8;
	UINT32 FagcLpfGainx:4;
	UINT32 IBRssi1:8;
	UINT32 Fagc2EqCal:1;
	UINT32 FgacCalGain:3;
} RX_VECTOR1_4TH_CYCLE;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR1_5TH_CYCLE {
	UINT32 LTF_SNR0:6;
	UINT32 LTF_ProcTime:7;
	UINT32 FoE:12; /* FreqOffSer from Rx */
	UINT32 CagcState:3;
	UINT32 FagcLnaGain1:2;
	UINT32 FagcLnaGain0:2;
} RX_VECTOR1_5TH_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR1_5TH_CYCLE {
	UINT32 FagcLnaGain0:2;
	UINT32 FagcLnaGain1:2;
	UINT32 CagcState:3;
	UINT32 FoE:12; /* FreqOffSer from Rx */
	UINT32 LTF_ProcTime:7;
	UINT32 LTF_SNR0:6;
} RX_VECTOR1_5TH_CYCLE;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR1_6TH_CYCLE {
	UINT32 Reserved:1;
	UINT32 CagcState:3;
	UINT32 NsTsField:3;
	UINT32 RxValidIndicator:1;
	UINT32 Nf2:8;
	UINT32 Nf1:8;
	UINT32 Nf0:8;
} RX_VECTOR1_6TH_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR1_6TH_CYCLE {
	UINT32 Nf0:8;
	UINT32 Nf1:8;
	UINT32 Nf2:8;
	UINT32 RxValidIndicator:1;
	UINT32 NsTsField:3;
	UINT32 CagcState:3;
	UINT32 Reserved:1;
} RX_VECTOR1_6TH_CYCLE;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR2_1ST_CYCLE {
	UINT32 Ofdm:28;
	UINT32 BtEnv:1;
	UINT32 Sec40ItfrEnv:1;
	UINT32 SecItfrEnv:1;
	UINT32 PrimItfrEnv:1;
} RX_VECTOR2_1ST_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR2_1ST_CYCLE {
	UINT32 PrimItfrEnv:1;
	UINT32 SecItfrEnv:1;
	UINT32 Sec40ItfrEnv:1;
	UINT32 BtEnv:1;
	UINT32 Ofdm:28;
} RX_VECTOR2_1ST_CYCLE;
#endif

#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RX_VECTOR2_2ND_CYCLE {
	UINT32 RxScramblingSeed:3;
	/* UINT32 DumpSelect:8; */
	UINT32 Reserved2:2;
	UINT32 OfdmLtfSNR1:6;
	UINT32 BtdNoTchLoc:7;
	UINT32 Ofdm:14;
} RX_VECTOR2_2ND_CYCLE;
#else
typedef struct GNU_PACKED _RX_VECTOR2_2ND_CYCLE {
	UINT32 Ofdm:14;
	UINT32 BtdNoTchLoc:7;
	/* UINT32 DumpSelect:8; */
	UINT32 OfdmLtfSNR1:6;
	UINT32 Reserved2:2;
	UINT32 RxScramblingSeed:3;
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
		UINT32 UnknownBit:1;/* TODO: Carter, check with WK for detail. */
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
		UINT32 UnknownBit:1;/* TODO: Carter, check with WK for detail. */
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

	struct wtbl_1_struc wtbl_1;
	struct wtbl_2_struc wtbl_2;
	union wtbl_3_struc wtbl_3;
	struct wtbl_4_struc wtbl_4;

	UINT32 wtbl_addr[4];
	UINT16 wtbl_fid[4];
	UINT16 wtbl_eid[4];
} WTBL_ENTRY;

#define MT_SMAC_BA_AGG_RANGE		8
#define MT_DMAC_BA_AGG_RANGE		8






#define MT_PSE_BASE_ADDR		0xa0000000

#define MT_TOP_REMAP_ADDR		0x80000000 /* TOP start address 8002-0000, but only can remap to 8000-0000 */
#define MT_TOP_REMAP_ADDR_THEMAL	 0xa2000 /* Get Thermal sensor adc cal value: 0x80022000 bits(8,14), Offset 0x80000 + 0x22000 = 0xa2000 */
#define MT_TOP_THEMAL_ADC	0x80022000 /* Get Thermal sensor adc cal value: 0x80022000 bits(8,14) */

#define MCU_CFG_BASE		0x2000
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

#define TOP_CFG_BASE        0x0000

#define XTAL_CTL4           (TOP_CFG_BASE + 0x1210)
#define XTAL_CTL13          (TOP_CFG_BASE + 0x1234)
#define XTAL_CTL14          (TOP_CFG_BASE + 0x1238)
#define DA_XO_C2_MASK (0x7f << 8)
#define DA_XO_C2(p) (((p) & 0x7f) << 8)


#define DBDC_BAND_NUM 1

#if defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA)
#define MT_PSE_PAGE_SIZE		256
#elif defined(MT7637_FPGA)
#define MT_PSE_PAGE_SIZE		128
#else
#define MT_PSE_PAGE_SIZE		128
#endif /* MT7603_FPGA */



#define MT_PCI_REMAP_ADDR_1		0x40000
#define MT_PCI_REMAP_ADDR_2		0x80000

/* TODO: shiang-7603, this is a dummy data structure and need to revise to adapative for MT7603 series */
#define WMM_QUE_NUM		4 /* each sta has 4 Queues to mapping to each WMM AC */

#ifndef COMPOS_WIN
/* value domain of pTxD->HostQId (4-bit: 0~15) */
#define QID_AC_BK               0
#define QID_AC_BE               1
#define QID_AC_VI               2
#define QID_AC_VO               3
#endif

#define QID_HCCA                4
#define NUM_OF_TX_RING		4
#define NUM_OF_WMM1_TX_RING	1

#define QID_BMC                 8

#define NUM_OF_RX_RING		1
#ifdef CONFIG_ANDES_SUPPORT
#undef NUM_OF_RX_RING
#define NUM_OF_RX_RING		2
#endif /* CONFIG_ANDES_SUPPORT */

#define QID_MGMT                13
#define QID_RX                  14
#define QID_OTHER               15
#ifdef CONFIG_ANDES_SUPPORT
#define QID_CTRL				16
#endif /* CONFIG_ANDES_SUPPORT */

#define QID_BCN				17

#define SHAREDKEYTABLE			0
#define PAIRWISEKEYTABLE		1

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

#define TSO_SIZE		0

#define EXP_ACK_TIME	0x1380


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

typedef struct __TX_CNT_INFO {
	union WTBL_2_DW5 wtbl_2_d5;
	union WTBL_2_DW6 wtbl_2_d6;
	union WTBL_2_DW7 wtbl_2_d7;
	union WTBL_2_DW8 wtbl_2_d8;
	union WTBL_2_DW9 wtbl_2_d9;
} TX_CNT_INFO;


#define E2PROM_CSR          0x0004
#define GPIO_CTRL_CFG	0x0228
#define WSC_HDR_BTN_GPIO_0			((UINT32)0x00000001) /* bit 0 for RT2860/RT2870 */
#define WSC_HDR_BTN_GPIO_3			((UINT32)0x00000008) /* bit 3 for RT2860/RT2870 */

#define LEGACY_BASIC_RATE	0x1408

struct _RTMP_ADAPTER;

VOID dump_mt_mac_cr(struct _RTMP_ADAPTER *pAd);

INT mt_wtbl_init_ByFw(struct _RTMP_ADAPTER *pAd);
INT mt_wtbl_init_ByDriver(struct _RTMP_ADAPTER *pAd);
INT mt_mac_init(struct _RTMP_ADAPTER *pAd);
INT mt_hw_tb_init(struct _RTMP_ADAPTER *pAd, BOOLEAN bHardReset);

INT mt_wtbl_get_entry234(struct _RTMP_ADAPTER *pAd, UCHAR idx, struct wtbl_entry *ent);
VOID dump_wtbl_entry(struct _RTMP_ADAPTER *pAd, struct wtbl_entry *ent);
VOID dump_wtbl_info(struct _RTMP_ADAPTER *pAd, UINT wtbl_idx);
#ifdef CONFIG_WTBL_TLV_MODE
VOID dump_wtbl_info_ByTlv(struct _RTMP_ADAPTER *pAd, UINT wtbl_idx);
#endif
VOID dump_wtbl_base_info(struct _RTMP_ADAPTER *pAd);

INT mt_mac_set_ctrlch(struct _RTMP_ADAPTER *pAd, UINT8 extch);
#ifdef GREENAP_SUPPORT
INT rtmp_mac_set_mmps(struct _RTMP_ADAPTER *pAd, INT ReduceCorePower);
#endif /* GREENAP_SUPPORT */

UINT16 tx_rate_to_tmi_rate(UINT8 mode, UINT8 mcs, UINT8 nss, BOOLEAN stbc, UINT8 preamble);
UCHAR get_nsts_by_mcs(UCHAR phy_mode, UCHAR mcs, BOOLEAN stbc, UCHAR vht_nss);

#define HIF_PORT 1
#define MCU_PORT 2
#define MT_TX_RETRY_UNLIMIT		0x1f
#define MT_TX_SHORT_RETRY		0x0f
#define MT_TX_LONG_RETRY		0x0f


VOID mt_chip_info_show(struct _RTMP_ADAPTER *pAd);
INT mt_nic_asic_init(struct _RTMP_ADAPTER *pAd);
VOID mt_rxv_set(struct _RTMP_ADAPTER *pAd, int DbgLvl);

struct _TX_BLK;
struct _MAC_TX_INFO;
union _HTTRANSMIT_SETTING;
VOID mt_write_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *buf, struct _TX_BLK *pTxBlk);
INT mt_hw_tx(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk);
INT mt_mlme_hw_tx(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info, struct _MAC_TX_INFO *info, HTTRANSMIT_SETTING *transmit, struct _TX_BLK *tx_blk);
#endif /* __MT_SMAC_H__ */

