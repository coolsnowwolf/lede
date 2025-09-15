#ifndef __A4_CONN_H__
#define __A4_CONN_H__
/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************


    Module Name:
	a4_conn.h

    Abstract:
    This is A4 connection function used to process those 4-addr of connected APClient or STA.
    Used by MWDS and MAP feature

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
#ifdef A4_CONN
#include "rtmp_def.h"

typedef struct _A4_CONNECT_ENTRY {
	DL_LIST List;
	UCHAR valid;
	UCHAR wcid;
} A4_CONNECT_ENTRY, *PA4_CONNECT_ENTRY;


#define A4_TYPE_NONE  0
#define A4_TYPE_MWDS  1
#define A4_TYPE_MAP   2 /*high priority*/


#define IS_ENTRY_A4(_x)					((_x)->a4_entry != 0)
#define GET_ENTRY_A4(_x)				((_x)->a4_entry)
#define SET_ENTRY_A4(_x, _type)			((_x)->a4_entry = _type)

#define IS_APCLI_A4(_x)					((_x)->a4_apcli != 0)
#define GET_APCLI_A4(_x)				((_x)->a4_apcli)
#define SET_APCLI_A4(_x, _type)			((_x)->a4_apcli = _type)


BOOLEAN a4_interface_init(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap,
	IN UCHAR a4_type
);

BOOLEAN a4_interface_deinit(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap,
	IN UCHAR a4_type
);

VOID a4_proxy_delete(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr
);

BOOLEAN a4_proxy_lookup(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr,
	IN BOOLEAN update_alive_time,
	IN BOOLEAN is_rx,
	OUT UCHAR *wcid
);

VOID a4_proxy_update(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN UCHAR wcid,
	IN PUCHAR mac_addr,
	IN UINT32 ip /* ARP Sender IP*/
);

VOID a4_proxy_maintain(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index
);

void a4_send_clone_pkt(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PNDIS_PACKET pkt,
	IN PUCHAR exclude_mac_addr
);

BOOLEAN a4_ap_peer_enable(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
);

BOOLEAN a4_ap_peer_disable(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
);

#ifdef APCLI_SUPPORT
BOOLEAN a4_apcli_peer_enable(
	IN PRTMP_ADAPTER adapter,
	IN PAPCLI_STRUCT apcli_entry,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
);

BOOLEAN a4_apcli_peer_disable(
	IN PRTMP_ADAPTER adapter,
	IN PAPCLI_STRUCT apcli_entry,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
);
#endif

INT Set_APProxy_Status_Show_Proc(
	IN  PRTMP_ADAPTER adapter,
	IN  RTMP_STRING *arg
);

INT Set_APProxy_Refresh_Proc(
	IN	PRTMP_ADAPTER adapter,
	IN	RTMP_STRING * arg);

#endif /* A4_CONN */
#endif /* __A4_CONN_H__*/

