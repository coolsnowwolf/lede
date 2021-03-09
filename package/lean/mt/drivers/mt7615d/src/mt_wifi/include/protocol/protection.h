 /***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2016, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************


    Module Name:
    protection.h

    Abstract:
    Generic 802.11 Legacy/HT/nonHT Protection Mechanism

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Hugo        2016-0505     create

*/

#ifndef __PROTECTION_H__
#define __PROTECTION_H__

struct _MAC_TABLE_ENTRY;
struct _BSS_STRUCT;

enum ht_protection {
	NON_PROTECT = 0,
	NONMEMBER_PROTECT = 1,
	BW20_PROTECT = 2,
	NONHT_MM_PROTECT = 3
};

enum peer_state {
	PEER_JOIN = 0,
	PEER_LEAVE
};

enum protection {
	/* 11n */
	NO_PROTECTION,
	NON_MEMBER_PROTECT,
	HT20_PROTECT,
	NON_HT_MIXMODE_PROTECT,
	_NOT_DEFINE_HT_PROTECT,
	/* b/g */
	ERP,
	/* vendor */
	LONG_NAV_PROTECT,
	RDG_PROTECT = LONG_NAV_PROTECT,
	GREEN_FIELD_PROTECT,
	RIFS_PROTECT,
	RDG,
	FORCE_RTS_PROTECT,
	_NOT_DEFINE_VENDOR_PROTECT,
	_END_PROTECT
};

#define SET_PROTECT(x)  (1 << (x))

#ifdef CONFIG_AP_SUPPORT

UINT16 nonerp_sta_num(struct _MAC_TABLE_ENTRY *peer, UCHAR peer_state);
UCHAR nonerp_protection(struct _BSS_STRUCT *bss_struct);

#endif

#endif /* __PROTECTION_H__ */
