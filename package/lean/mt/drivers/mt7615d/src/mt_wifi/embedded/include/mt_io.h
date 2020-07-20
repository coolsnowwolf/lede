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
	mt_io.h
*/

#ifndef __MT_WIFI_IO_H__
#define __MT_WIFI_IO_H__

struct _RTMP_ADAPTER;

UINT32 mt_physical_addr_map(struct _RTMP_ADAPTER *pAd, UINT32 addr);
BOOLEAN mt_mac_cr_range_mapping(struct _RTMP_ADAPTER *pAd, UINT32 *mac_addr);

#endif
