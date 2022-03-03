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
	mac.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __MAC_H__
#define __MAC_H__

#ifdef MT_MAC
#include "mac/mac_mt/mt_mac.h"
#endif /* MT_MAC */

#if defined(RTMP_MAC) || defined(RLT_MAC)
#include "mac/mac_ral/pbf.h"
#include "mac/mac_ral/rf_ctrl.h"
#include "mac/mac_ral/rtmp_mac.h"
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */


#define TXINFO_SIZE			4
typedef union GNU_PACKED _TXINFO_STRUC{
#ifdef RTMP_MAC
	struct _TXINFO_OMAC txinfo_omac;
#endif /* RTMP_MAC */
#ifdef RLT_MAC
	struct _TXINFO_NMAC_PKT txinfo_nmac_pkt;
	struct _TXINFO_NMAC_CMD txinfo_nmac_cmd;
#endif /* RLT_MAC */
	UINT32 word;
}TXINFO_STRUC;

#define SHORT_PREAMBLE 0
#define LONG_PREAMBLE 1

/*
	bit31 =>802.3 if set 1, implay you hav did header translation
	bit30 => put VLAN field
	
*/
#define RAL_RXINFO_SIZE			4
#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _RXINFO_STRUC {
	UINT32		hdr_trans_ip_sum_err:1;		/* IP checksum error */
	UINT32		vlan_taged_tcp_sum_err:1;	/* TCP checksum error */
	UINT32		rsv:1;
	UINT32		action_wanted:1;
	UINT32		deauth:1;
	UINT32		disasso:1;
	UINT32		beacon:1;
	UINT32		probe_rsp:1;
	UINT32		sw_fc_type1:1;
	UINT32		sw_fc_type0:1;
	UINT32		pn_len:3;
	UINT32		wapi_kidx:1;
	UINT32		BssIdx3:1;
	UINT32		Decrypted:1;
	UINT32		AMPDU:1;
	UINT32		L2PAD:1;
	UINT32		RSSI:1;
	UINT32		HTC:1;
	UINT32		AMSDU:1;		/* rx with 802.3 header, not 802.11 header. obsolete. */
	UINT32		CipherErr:2;        /* 0: decryption okay, 1:ICV error, 2:MIC error, 3:KEY not valid */
	UINT32		Crc:1;			/* 1: CRC error */
	UINT32		MyBss:1;		/* 1: this frame belongs to the same BSSID */
	UINT32		Bcast:1;			/* 1: this is a broadcast frame */
	UINT32		Mcast:1;			/* 1: this is a multicast frame */
	UINT32		U2M:1;			/* 1: this RX frame is unicast to me */
	UINT32		FRAG:1;
	UINT32		NULLDATA:1;
	UINT32		DATA:1;
	UINT32		BA:1;
}	RXINFO_STRUC;
#else
typedef	struct GNU_PACKED _RXINFO_STRUC {
	UINT32		BA:1;
	UINT32		DATA:1;
	UINT32		NULLDATA:1;
	UINT32		FRAG:1;
	UINT32		U2M:1;
	UINT32		Mcast:1;
	UINT32		Bcast:1;
	UINT32		MyBss:1;
	UINT32		Crc:1;
	UINT32		CipherErr:2;
	UINT32		AMSDU:1;
	UINT32		HTC:1;
	UINT32		RSSI:1;
	UINT32		L2PAD:1;
	UINT32		AMPDU:1;
	UINT32		Decrypted:1;
	UINT32		BssIdx3:1;
	UINT32		wapi_kidx:1;
	UINT32		pn_len:3;
	UINT32		sw_fc_type0:1;
	UINT32      sw_fc_type1:1;
	UINT32      probe_rsp:1;
	UINT32		beacon:1;
	UINT32		disasso:1;
	UINT32      deauth:1;
	UINT32      action_wanted:1;
	UINT32      rsv:1;
	UINT32		vlan_taged_tcp_sum_err:1;
	UINT32		hdr_trans_ip_sum_err:1;
}RXINFO_STRUC;
#endif


typedef struct _MAC_TX_INFO{
	UCHAR WCID;
	BOOLEAN FRAG;
	BOOLEAN InsTimestamp;
	BOOLEAN NSeq;
	BOOLEAN Ack;
	BOOLEAN BM;
	BOOLEAN CFACK;
	BOOLEAN AMPDU;
	UCHAR BASize;
	UCHAR PID;
	UCHAR TID;
	UCHAR TxRate;
	UCHAR Txopmode;
	ULONG Length;
	UCHAR hdr_len;
	UCHAR hdr_pad;
	UCHAR eth_type_offset;
	UCHAR bss_idx;
	UCHAR q_idx;
	UCHAR prot;
	UCHAR AntPri;
	UCHAR SpeEn;
	UCHAR Preamble;
#ifdef MT_MAC
	UINT32 TxSPriv;
	UCHAR PsmBySw; /* PSM bit controlled by SW */
#endif /* MT_MAC */
}MAC_TX_INFO;


struct _RTMP_ADAPTER;
#ifdef DBG
VOID dump_txinfo(struct _RTMP_ADAPTER *pAd, TXINFO_STRUC *pTxInfo);
VOID dump_rxinfo(struct _RTMP_ADAPTER *pAd, RXINFO_STRUC *pRxInfo);

VOID dump_tmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *tmac_info);
VOID dump_rmac_info(struct _RTMP_ADAPTER *pAd, UCHAR *rmac_info);

#ifdef RTMP_PCI_SUPPORT
VOID dump_txd(struct _RTMP_ADAPTER *pAd, struct _TXD_STRUC *pTxD);
VOID dump_rxd(struct _RTMP_ADAPTER *pAd, struct _RXD_STRUC *pRxD);
#endif
#endif /* DBG */
#endif /* __MAC_H__ */

