/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

        Abstract:

        All related IEEE802.11f IAPP + IEEE802.11r IAPP extension.

***************************************************************************/

#ifndef __RT_CONFIG_H__
#define __RT_CONFIG_H__

/* Include Kernel file */
#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/wireless.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "rt_typedef.h"

/* Definition of OID to RALINK AP driver */
#define OID_GET_SET_TOGGLE 0x8000
#define RT_QUERY_SIGNAL_CONTEXT 0x0402
#define RT_SET_IAPP_PID 0x0404
#define RT_SET_APD_PID 0x0405
#define RT_SET_DEL_MAC_ENTRY 0x0406
#define RT_QUERY_EVENT_TABLE 0x0407

#define RT_SET_FT_STATION_NOTIFY 0x0408
#define RT_SET_FT_KEY_REQ 0x0409
#define RT_SET_FT_KEY_RSP 0x040a

#define RT_FT_KEY_SET 0x040b
#define RT_FT_DATA_ENCRYPT 0x040c
#define RT_FT_DATA_DECRYPT 0x040d
#define RT_FT_NEIGHBOR_REPORT 0x040e
#define RT_FT_NEIGHBOR_REQUEST 0x040f
#define RT_FT_NEIGHBOR_RESPONSE 0x0410
#define RT_FT_ACTION_FORWARD 0x0411

/* File path */
// #define MSG_FILE "/etc/wireless/mt7615/mtkiappd.dat"
#define EVENT_LOG_FILE "/var/log/mtkiappd.log"

/* ReDefinition */
#define NdisZeroMemory(__Dst, __Len) memset(__Dst, 0, __Len)
#define NdisFillMemory(__Dst, __Len, __Val) memset(__Dst, __Val, __Len)
#define NdisMoveMemory(__Dst, __Src, __Len) memmove(__Dst, __Src, __Len)
#define NdisCopyMemory(__Dst, __Src, __Len) memcpy(__Dst, __Src, __Len)
#define NdisCompareMemory(__Dst, __Src, __Len) memcmp(__Dst, __Src, __Len)

VOID os_alloc_mem(UCHAR *pAd, UCHAR **ppMem, UINT32 Size);
VOID os_free_mem(UCHAR *pAd, VOID *pMem);

/* Debug flag */
#define RT_DEBUG_OFF 0
#define RT_DEBUG_ERROR 1
#define RT_DEBUG_WARN 2
#define RT_DEBUG_TRACE 3
#define RT_DEBUG_INFO 4

#define NIC_DBG_STRING (" ")

#ifdef DBG
#define DBGPRINT(Level, Fmt)                                                   \
  {                                                                            \
    if (Level <= RTDebugLevel) {                                               \
      printf Fmt;                                                              \
    }                                                                          \
  }
#else

/* no debug information */
#define DBGPRINT(Level, Fmt)
#endif

#define MAX_NUM_OF_EVENT 30 /* entry # in EVENT table */

typedef struct _RT_802_11_EVENT_LOG {

  ULONG SystemTime; /* timestammp (jiffies) */
  UCHAR TriggerAddr[ETH_ALEN];
  UCHAR DetectorAddr[ETH_ALEN];
  UINT16 Event; /* EVENT_xxx */
} RT_802_11_EVENT_LOG, *PRT_802_11_EVENT_LOG;

typedef struct _RT_802_11_EVENT_TABLE {

  ULONG Num;
  RT_802_11_EVENT_LOG Log[MAX_NUM_OF_EVENT];
} RT_802_11_EVENT_TABLE, *PRT_802_11_EVENT_TABLE;

#define PACKED __attribute__((packed))

typedef struct PACKED _FT_KDP_EVT_HEADER {

  UINT32 EventLen;
  UINT32 PeerIpAddr;

} FT_KDP_EVT_HEADER;

typedef struct PACKED _RT_SIGNAL_STRUC {

  VOID *pNext; /* point to next signal */

  UINT16 Sequence;
  UCHAR MacAddr[ETH_ALEN];
  UCHAR CurrAPAddr[ETH_ALEN];

#define FT_KDP_SIG_NOTHING 0x00            /* no signal */
#define FT_KDP_SIG_IAPP_ASSOCIATION 0x01   /* A station has associated */
#define FT_KDP_SIG_IAPP_REASSOCIATION 0x02 /* A station has re-associated */
#define FT_KDP_SIG_TERMINATE 0x03          /* terminate the daemon */

#define FT_KDP_SIG_FT_ASSOCIATION 0x50   /* A FT station has associated */
#define FT_KDP_SIG_FT_REASSOCIATION 0x51 /* A FT station has re-associated */
#define FT_KDP_SIG_KEY_TIMEOUT 0x52      /* PMK-R1 KEY Timeout */
#define FT_KDP_SIG_KEY_REQ 0x53          /* Request PMK-R1 KEY from R0KH */
#define FT_KDP_SIG_ACTION 0x54           /* Forward FT Action frame to DS */

#define FT_KDP_SIG_AP_INFO_REQ 0x70 /* request neighbor AP info. */
#define FT_KDP_SIG_AP_INFO_RSP 0x71 /* response my AP info. */

/* FT KDP internal use */
#define FT_KDP_SIG_KEY_REQ_AUTO 0xA0   /* Request PMK-R1 KEY from R0KH */
#define FT_KDP_SIG_KEY_RSP_AUTO 0xA1   /* Response PMK-R1 KEY to R1KH */
#define FT_KDP_SIG_INFO_BROADCAST 0xB0 /* broadcast our AP information */

#define FT_KSP_SIG_DEBUG_TRACE 0xC0 /* enable debug flag to TRACE */
  UCHAR Sig;

  UCHAR MacAddrSa[ETH_ALEN];

  /* IEEE80211R_SUPPORT */
  /* the first 6B are FT_KDP_EVT_HEADER */
  /*
          For FT_KDP_SIG_NOTHING:			nothing
          For FT_KDP_SIG_IAPP_ASSOCIATION:nothing
          For FT_KDP_SIG_REASSOCIATION:	nothing
          For FT_KDP_SIG_TERMINATE:		nothing
          For FT_KDP_SIG_FT_ASSOCIATION:	nothing
          FT_KDP_SIG_FT_REASSOCIATION:	nothing
          For FT_KDP_SIG_KEY_TIMEOUT:		it is
          For FT_KDP_SIG_KEY_REQ_AUTO:	it is FT_KDP_EVT_KEY_REQ
          For FT_KDP_SIG_KEY_RSP_AUTO:	it is FT_KDP_SIG_KEY_RSP
  */
  UCHAR Reserved[3];   /* let Content address four-byte align */
  UCHAR Content[1024]; /* signal content */

#define RT_SIGNAL_STRUC_HDR_SIZE (sizeof(RT_SIGNAL_STRUC) - 1024)
} RT_SIGNAL_STRUC, *PRT_SIGNAL_STRUC;

static INT32 RTDebugLevel = RT_DEBUG_ERROR;

#endif /* __RT_CONFIG_H__ */

/* End of rt_config.h */
