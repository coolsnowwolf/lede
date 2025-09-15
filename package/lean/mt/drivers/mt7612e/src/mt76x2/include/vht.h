/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/

#include "dot11ac_vht.h"


struct _RTMP_ADAPTER;
struct _RT_PHY_INFO;


VOID dump_vht_cap(struct _RTMP_ADAPTER *pAd, VHT_CAP_IE *vht_ie);
VOID dump_vht_op(struct _RTMP_ADAPTER *pAd, VHT_OP_IE *vht_ie);

INT build_vht_txpwr_envelope(struct _RTMP_ADAPTER *pAd, UCHAR *buf);
INT build_vht_ies(struct _RTMP_ADAPTER *pAd, UCHAR *buf, UCHAR frm);
INT build_vht_cap_ie(struct _RTMP_ADAPTER *pAd, UCHAR *buf);

UCHAR vht_prim_ch_idx(UCHAR vht_cent_ch, UCHAR prim_ch);
UCHAR vht_cent_ch_freq(struct _RTMP_ADAPTER *pAd, UCHAR prim_ch);
INT vht_mode_adjust(struct _RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, VHT_CAP_IE *cap, VHT_OP_IE *op);
INT SetCommonVHT(struct _RTMP_ADAPTER *pAd);
VOID rtmp_set_vht(struct _RTMP_ADAPTER *pAd, struct _RT_PHY_INFO *phy_info);

#ifdef VHT_TXBF_SUPPORT
VOID trigger_vht_ndpa(struct _RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *entry);
#endif /* VHT_TXBF_SUPPORT */

void assoc_vht_info_debugshow(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN VHT_CAP_IE *vht_cap,
	IN VHT_OP_IE *vht_op);

BOOLEAN vht80_channel_group( struct _RTMP_ADAPTER *pAd, UCHAR channel);

