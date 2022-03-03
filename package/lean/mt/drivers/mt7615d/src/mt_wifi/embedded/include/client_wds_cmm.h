/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	client_wds_cmm.h

	Abstract:
*/

#ifndef __CLIENT_WDS_CMM_H__
#define __CLIENT_WDS_CMM_H__

#include "rtmp_def.h"

#ifdef CLIENT_WDS


#ifdef MBSS_AS_WDS_AP_SUPPORT
#define CLI_WDS_ENTRY_AGEOUT 300000  /* 300 seconds */
#else
#define CLI_WDS_ENTRY_AGEOUT 5000  /* seconds */
#endif


#define CLIWDS_POOL_SIZE 128
#define CLIWDS_HASH_TAB_SIZE 64  /* the legth of hash table must be power of 2. */
typedef struct _CLIWDS_PROXY_ENTRY {
	struct _CLIWDS_PROXY_ENTRY *pNext;
	ULONG LastRefTime;
	SHORT Aid;
	UCHAR Addr[MAC_ADDR_LEN];
} CLIWDS_PROXY_ENTRY, *PCLIWDS_PROXY_ENTRY;

#endif /* CLIENT_WDS */

#endif /* __CLIENT_WDS_CMM_H__ */

