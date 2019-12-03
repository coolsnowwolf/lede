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
	wapp.h
*/
#include "rt_config.h"

#ifndef _WAPP_H_
#define __WAPP_H__

#ifdef WAPP_SUPPORT
struct wapp_req;
struct wapp_event;

INT	wapp_event_handle(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req);

INT wapp_send_cli_join_event(
	PRTMP_ADAPTER pAd,
	MAC_TABLE_ENTRY *mac_entry);

INT wapp_send_cli_leave_event(
	PRTMP_ADAPTER pAd,
	UINT32 ifindex,
	UCHAR *mac_addr);

INT wapp_send_cli_probe_event(
	PRTMP_ADAPTER pAd,
	UINT32 ifindex,
	UCHAR *mac_addr,
	MLME_QUEUE_ELEM *elem);

BOOLEAN wapp_init(
	PRTMP_ADAPTER pAd,
	BSS_STRUCT *pMbss);

VOID wext_send_wapp_qry_rsp(
	PNET_DEV pNetDev,
	struct wapp_event *event);

INT wapp_send_bss_stop_rsp(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev
);

INT wapp_send_ch_change_rsp(
	PRTMP_ADAPTER pAd,
	MT_SWITCH_CHANNEL_CFG SwChCfg
);

INT wapp_send_apcli_association_change(
	u8 event_id,
	PRTMP_ADAPTER pAd,
	PAPCLI_STRUCT pApCliEntry
);

VOID wapp_send_bcn_report(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PUCHAR pFramePtr,
	IN ULONG MsgLen
);

VOID wapp_send_bcn_report_complete(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry
);

#ifdef AIR_MONITOR
VOID wapp_send_air_mnt_rssi(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PMNT_STA_ENTRY pMntEntry);
#endif

VOID wapp_send_cli_active_change(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry);

#endif /* WAPP_SUPPORT */
#endif /* _WAPP_H_ */

