#ifndef __SNIFFER_MIB_H__
#define __SNIFFER_MIB_H__

#include "link_list.h"

//this use the MAC Repeater's  function : MT7603's MAX is 16
#define MAX_NUM_OF_SNIFFER_MAC	16

#ifdef CONFIG_SNIFFER_SUPPORT
#ifdef ALL_NET_EVENT
#define SNIFFER_MAC_MAX_RX_PACKET_COUNT 20
#define SNIFFER_MAC_TIMEOUT 10000 //ms

typedef struct _SNIFFER_MAC_NOTIFY {
	UINT32 				u4Channel;
	INT32 				i4RxPacketConut;
	INT32				i4RssiAccum;
	PNET_DEV 			pNetDev;
} SNIFFER_MAC_NOTIFY_T, *PSNIFFER_MAC_NOTIFY_T;
#endif /* ALL_NET_EVENT */
#endif /* CONFIG_SNIFFER_SUPPORT */


typedef struct _SNIFFER_MAC_CTRL {
	DL_LIST List;
	UCHAR MACAddr[MAC_ADDR_LEN];
#ifdef CONFIG_SNIFFER_SUPPORT
#ifdef ALL_NET_EVENT
	SNIFFER_MAC_NOTIFY_T rNotify;
	RALINK_TIMER_STRUCT rNotifyTimer;
#endif /* ALL_NET_EVENT */
#endif /* CONFIG_SNIFFER_SUPPORT */
} SNIFFER_MAC_CTRL, *PSNIFFER_MAC_CTRL;

typedef enum _SNIFFER_SCAN_SCOPE {
	SCAN_ALL, /* no need */
	SCAN_ONE,
} SNIFFER_SCAN_SCOPE;

typedef enum _SNIFFER_SCAN_TYPE {
	UNKNOWN_TYPE,
	PASSIVE_TYPE,
	ACTIVE_TYPE, /* no need */
} SNIFFER_SCAN_TYPE;

typedef struct _SNIFFER_MIB_CTRL {
	UINT32 MAC_ListNum;
	PRTMP_ADAPTER pAd;
	SNIFFER_SCAN_SCOPE scan_scope;
	SNIFFER_SCAN_TYPE scan_type;
	UINT32 scan_channel; // which channel to sniffer
		
	UINT8 bbp_bw; // keep original pAd->hw_cfg.bbp_bw
	UCHAR vht_cent_ch; //keep original pAd->CommonCfg.vht_cent_ch
	UCHAR CentralChannel; //kepp original pAd->CommonCfg.CentralChannel
	UCHAR Channel;// keep original pAd->CommonCfg.Channel
		
	ULONG AgeOutTime; // ms for AgeOutTimer
	RALINK_TIMER_STRUCT AgeOutTimer;
	BOOLEAN AgeOutTimer_Running;
	RTMP_OS_SEM MAC_ListLock;
	DL_LIST MAC_List;
	BOOLEAN CheckAllMAC;
} SNIFFER_MIB_CTRL, *PSNIFFER_MIB_CTRL;

INT sniffer_channel_restore(IN RTMP_ADAPTER *pAd);

VOID sniffer_mib_ctrlInit(IN PRTMP_ADAPTER pAd);
VOID sniffer_mib_ctrlExit(IN PRTMP_ADAPTER pAd);
	
INT exsta_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg);
INT exsta_list_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg);
INT exsta_clear_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg);
INT exsta_scan_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg);

INT set_mib_passive_scan_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg);
INT set_mib_scan_interval_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg);
INT set_mib_scan_scope_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg);
INT set_mib_scan_channel_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg);

#endif /* __SNIFFER_MIB_H__ */
