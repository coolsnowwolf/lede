/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	wifi_sys_info.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __WIFI_SYS_INFO_H__
#define __WIFI_SYS_INFO_H__

#include "common/link_list.h"
#include "wifi_sys_notify.h"
#include "mgmt/be_export.h"

struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _IE_lists;
struct _STA_TR_ENTRY;
struct _IE_lists;

#ifdef APCLI_SUPPORT
struct _APCLI_STRUCT;
#endif

#define WIFI_SYS_POLL_MAX 10
#define WIFI_SYS_PILL_PERIOD 100

enum {
	WSYS_NOTIFY_OPEN,
	WSYS_NOTIFY_CLOSE,
	WSYS_NOTIFY_CONNT_ACT,
	WSYS_NOTIFY_DISCONNT_ACT,
	WSYS_NOTIFY_LINKUP,
	WSYS_NOTIFY_LINKDOWN,
	WSYS_NOTIFY_STA_UPDATE,
};

enum {
	WSYS_NOTIFY_PRIORITY_DEFAULT = 0,
	WSYS_NOTIFY_PRIORITY_QM,
	WSYS_NOTIFY_PRIORITY_MLME,
};

typedef struct _DEV_INFO_CTRL_T {
	UINT8 OwnMacIdx;
	UINT8 OwnMacAddr[MAC_ADDR_LEN];
	UINT8 BandIdx;
	UINT8 Active;
	UINT32 EnableFeature;
	VOID *priv;
	DL_LIST list;
} DEV_INFO_CTRL_T;


typedef struct _STA_REC_CTRL_T {
	UINT8 BssIndex;
	UINT8 WlanIdx;
	UINT32 ConnectionType;
	UINT8 ConnectionState;
	UINT32 EnableFeature;
	UINT8 IsNewSTARec;
	ASIC_SEC_INFO asic_sec_info;
	VOID *priv;
	DL_LIST list;
} STA_REC_CTRL_T;


struct _tx_burst_cfg {
	struct wifi_dev *wdev;
	UINT8 prio;
	UINT8 ac_type;
	UINT16 txop_level;
	UINT8 enable;
};

/*use for update bssinfo*/
struct uapsd_config {
	BOOLEAN uapsd_en;
	UCHAR uapsd_trigger_ac;
};

extern UINT16 txop0;
extern UINT16 txop60;
extern UINT16 txop80;
extern UINT16 txopfe;


#define TXOP_0          (txop0)
#define TXOP_30         (0x30)
#define TXOP_60         (txop60)
#define TXOP_80         (txop80)
#define TXOP_A0         (0xA0)
#define TXOP_C0         (0xC0)
#define TXOP_FE         (txopfe)
#define TXOP_138        (0x138)
#define TXOP_177        (0x177)

enum _tx_burst_prio {
	PRIO_DEFAULT = 0,
	PRIO_RDG,
	PRIO_MULTI_CLIENT,
	PRIO_2G_INFRA,
	PRIO_MU_MIMO,
	PRIO_PEAK_TP,
	PRIO_APCLI_REPEATER,
	PRIO_CCI,
	PRIO_WMM,
	MAX_PRIO_NUM
};

#define WDEV_BSS_STATE(__wdev) \
	(__wdev->bss_info_argument.bss_state)

typedef enum _BSSINFO_LINK_TO_OMAC_T {
	HW_BSSID = 0,
	EXTEND_MBSS,
	WDS,
	REPT
} BSSINFO_TYPE_T;

typedef enum _BSSINFO_STATE_T {
	BSS_INIT	= 0,	/* INIT state */
	BSS_INITED	= 1,	/* BSS Argument Link done */
	BSS_ACTIVE	= 2,	/* The original flag - Active */
	BSS_READY	= 3		/* BssInfo updated to FW done and ready for beaconing */
} BSSINFO_STATE_T;

typedef struct _BSS_INFO_ARGUMENT_T {
	BSSINFO_TYPE_T bssinfo_type;
	BSSINFO_STATE_T bss_state;
	UCHAR OwnMacIdx;
	UINT8 ucBssIndex;
	UINT8 Bssid[MAC_ADDR_LEN];
	UINT8 ucBcMcWlanIdx;
	UINT8 ucPeerWlanIdx;
	UINT32 NetworkType;
	UINT32 u4ConnectionType;
	UINT8 CipherSuit;
	UINT8 WmmIdx;
	UINT8 ucPhyMode;
	UINT32 prio_bitmap;
	UINT16 txop_level[MAX_PRIO_NUM];
	UINT32 u4BssInfoFeature;
	CMD_BSSINFO_PM_T rBssInfoPm;
	HTTRANSMIT_SETTING BcTransmit;
	HTTRANSMIT_SETTING McTransmit;
	UINT16	bcn_period;
	UINT8	dtim_period;
	struct freq_oper chan_oper;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	RA_COMMON_INFO_T ra_cfg;
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/
	VOID *priv;
	DL_LIST list;
} BSS_INFO_ARGUMENT_T, *PBSS_INFO_ARGUMENT_T;

struct WIFI_SYS_CTRL {
	struct wifi_dev *wdev;
	DEV_INFO_CTRL_T DevInfoCtrl;
	STA_REC_CTRL_T StaRecCtrl;
	BSS_INFO_ARGUMENT_T BssInfoCtrl;
	VOID *priv;
	BOOLEAN skip_set_txop;
};

typedef struct _WIFI_INFO_CLASS {
	UINT32 Num;
	DL_LIST Head;
} WIFI_INFO_CLASS_T;

struct wsys_notify_info {
	struct wifi_dev *wdev;
	void *v;
};

/*for FW related information sync.*/
typedef struct _WIFI_SYS_INFO {
	WIFI_INFO_CLASS_T DevInfo;
	WIFI_INFO_CLASS_T BssInfo;
	WIFI_INFO_CLASS_T StaRec;
	struct notify_head wsys_notify_head;
	NDIS_SPIN_LOCK lock;
} WIFI_SYS_INFO_T;

typedef struct _PEER_LINKUP_HWCTRL {
#ifdef TXBF_SUPPORT
	struct _IE_lists ie_list;
	BOOLEAN bMu;
	BOOLEAN bETxBf;
	BOOLEAN bITxBf;
	BOOLEAN bMuTxBf;
#endif /*TXBF_SUPPORT*/
	BOOLEAN bRdgCap;
} PEER_LINKUP_HWCTRL;

typedef struct _LINKUP_HWCTRL {
#ifdef TXBF_SUPPORT
	BOOLEAN bMu;
	BOOLEAN bETxBf;
	BOOLEAN bITxBf;
	BOOLEAN bMuTxBf;
#endif /*TXBF_SUPPORT*/
	BOOLEAN bRdgCap;
} LINKUP_HWCTRL;

/*Export function*/
VOID wifi_sys_reset(struct _WIFI_SYS_INFO *wsys);
VOID wifi_sys_init(struct _RTMP_ADAPTER *ad);
VOID wifi_sys_dump(struct _RTMP_ADAPTER *ad);
INT wifi_sys_update_devinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _DEV_INFO_CTRL_T *devinfo);
INT wifi_sys_update_bssinfo(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, BSS_INFO_ARGUMENT_T *bss);
INT wifi_sys_update_starec(struct _RTMP_ADAPTER *ad, struct _STA_REC_CTRL_T *new);
INT wifi_sys_update_starec_info(struct _RTMP_ADAPTER *ad, struct _STA_REC_CTRL_T *new);
INT register_wsys_notifier(struct _WIFI_SYS_INFO *wsys, struct notify_entry *ne);
INT unregister_wsys_notifier(struct _WIFI_SYS_INFO *wsys, struct notify_entry *ne);
struct _STA_REC_CTRL_T *get_starec_by_wcid(struct _RTMP_ADAPTER *ad, INT wcid);

#ifdef CONFIG_AP_SUPPORT
#endif /*CONFIG_AP_SUPPORT*/

#ifdef APCLI_SUPPORT
#endif /*APCLI*/

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
VOID WifiSysRaInit(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
VOID WifiSysUpdateRa(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry, struct _STAREC_AUTO_RATE_UPDATE_T *prParam);
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/

VOID WifiSysUpdatePortSecur(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry, ASIC_SEC_INFO *asic_sec_info);
/*wifi system architecture layer api*/
INT wifi_sys_open(struct wifi_dev *wdev);
INT wifi_sys_close(struct wifi_dev *wdev);
INT wifi_sys_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT wifi_sys_disconn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT wifi_sys_linkup(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT wifi_sys_linkdown(struct wifi_dev *wdev);
VOID wifi_sys_ops_register(struct wifi_dev *wdev);
VOID wifi_sys_update_wds(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif
