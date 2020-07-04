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
	band_steering_def.h
*/

#ifndef _BAND_STEERING_DEF_H_
#define _BAND_STEERING_DEF_H_

#ifdef BAND_STEERING
#ifndef DOT11_N_SUPPORT
#error: "DOT11_N_SUPPORT must be enabled when using band steering"
#endif /* DOT11_N_SUPPORT */

#define BND_STRG_MAX_TABLE_SIZE	64
#define P_BND_STRG_TABLE(_x)	(&pAd->ApCfg.BndStrgTable[_x])
/* #define SIZE_OF_VHT_CAP_IE		12 */
#define IS_5G_BAND(_p)			(((_p)&BAND_5G) == BAND_5G)

#define BND_STRG_DBG
#define BND_STRG_QA

typedef struct _BND_STRG_CLI_ENTRY {
	BOOLEAN bValid;
	BOOLEAN bConnStatus;
	UINT8	TableIndex;
	UINT32 Control_Flags;
	ULONG   jiffies;		/* timestamp when insert-entry */
	UINT32  elapsed_time; /* ms */
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR BndStrg_Sta_State;
	struct _BND_STRG_CLI_ENTRY *pNext;
} BND_STRG_CLI_ENTRY, *PBND_STRG_CLI_ENTRY;

/* WPS_BandSteering Support */
typedef struct _WPS_WHITELIST_ENTRY {
	struct _WPS_WHITELIST_ENTRY *pNext;
	UCHAR addr[MAC_ADDR_LEN];
	UCHAR state;
} WPS_WHITELIST_ENTRY, *PWPS_WHITELIST_ENTRY;

typedef struct _BS_LIST_ENTRY {
	struct _BS_LIST_ENTRY *pNext;
	UCHAR addr[MAC_ADDR_LEN];
	UCHAR state;
} BS_LIST_ENTRY, *PBS_LIST_ENTRY;

#define NVRAM_TABLE_SIZE		128

typedef struct _bndstrg_nvram_client {
	UINT8 Addr[MAC_ADDR_LEN];
	UINT8 Manipulable;
	UINT8 PhyMode;
	UINT8 Band;
	UINT8 Nss;
} BNDSTRG_NVRAM_CLIENT, *PBNDSTRG_NVRAM_CLIENT;

typedef struct _bndstrg_nvram_list {
	UINT8			Num;
	BNDSTRG_NVRAM_CLIENT nvram_entry[NVRAM_TABLE_SIZE];
} BNDSTRG_NVRAM_LIST, *PBNDSTRG_NVRAM_LIST;

enum PhyMode {
	fPhyMode_Legacy,
	fPhyMode_HT,
	fPhyMode_VHT,
};

#define BND_STRG_MAX_WHITELIST_ENTRY	16
#define BND_STRG_MAX_BLACKLIST_ENTRY	4

typedef struct _BND_STRG_CLI_TABLE {
	BOOLEAN bInitialized;
	BOOLEAN bEnabled;
	UINT32 Size;
	BND_STRG_CLI_ENTRY Entry[BND_STRG_MAX_TABLE_SIZE];
	PBND_STRG_CLI_ENTRY Hash[HASH_TABLE_SIZE];
	NDIS_SPIN_LOCK Lock;
	VOID *priv;
	BOOLEAN bInfReady;
	CHAR	ucIfName[32];
	UINT8	uIdx;
#ifdef BND_STRG_DBG
	UCHAR MonitorAddr[MAC_ADDR_LEN];
#endif /* BND_STRG_DBG */
#ifdef VENDOR_FEATURE5_SUPPORT
	BNDSTRG_NVRAM_CLIENT nvram_entry[NVRAM_TABLE_SIZE];
	UINT8 bndstrg_nvram_client_count;
#endif /* VENDOR_FEATURE5_SUPPORT */
	UINT8		Band;
	UINT8		Channel;
	BOOLEAN     bVHTCapable;
	UINT8		nss;
	INT8		ActiveCount;
	UINT32		DaemonPid;
#ifdef DOT11K_RRM_SUPPORT
	RRM_NEIGHBOR_REP_INFO NeighborRepInfo;
#endif
	UINT8		BndStrgMode;
/* WPS_BandSteering Support */
	LIST_HEADER WpsWhiteList;	/* WPS: init in bandstearing table init */
	NDIS_SPIN_LOCK WpsWhiteListLock;
	LIST_HEADER WhiteList;	/* init in bandstearing table init */
	NDIS_SPIN_LOCK WhiteListLock;
	LIST_HEADER BlackList;	/* init in bandstearing table init */
	NDIS_SPIN_LOCK BlackListLock;
} BND_STRG_CLI_TABLE, *PBND_STRG_CLI_TABLE;

enum BND_STRG_RETURN_CODE {
	BND_STRG_SUCCESS = 0,
	BND_STRG_INVALID_ARG,
	BND_STRG_RESOURCE_ALLOC_FAIL,
	BND_STRG_TABLE_FULL,
	BND_STRG_TABLE_IS_NULL,
	BND_STRG_NOT_INITIALIZED,
	BND_STRG_2G_INF_NOT_READY,
	BND_STRG_5G_INF_NOT_READY,
	BND_STRG_STA_IS_CONNECTED,
	BND_STRG_UNEXP,
};

enum ACTION_CODE {
	CLI_EVENT = 1,
	CLI_ADD,
	CLI_DEL,
	CLI_STATUS_REQ,
	CLI_STATUS_RSP,
	CHANLOAD_STATUS_REQ,
	CHANLOAD_STATUS_RSP,
	INF_STATUS_QUERY,
	INF_STATUS_RSP,
	TABLE_INFO,
	ENTRY_LIST,
	BNDSTRG_ONOFF,
	SET_MNT_ADDR,
	NVRAM_UPDATE,
	REJECT_EVENT,
	HEARTBEAT_MONITOR,
	BNDSTRG_WNM_BTM,
	BNDSTRG_PARAM,
	BNDSTRG_NEIGHBOR_REPORT,
	UPDATE_WHITE_BLACK_LIST,
};

enum BND_STRG_STA_STATE {
	BNDSTRG_STA_INIT = 0,
	BNDSTRG_STA_ASSOC,
	BNDSTRG_STA_DISASSOC,
};

enum bndstrg_mode{
	PRE_CONNECTION_STEERING	= 0x01,
	POST_CONNECTION_STEERING = 0x02,
};

enum bndstrg_list_type {
	bndstrg_whitelist	=	1,
	bndstrg_blacklist	=	2,
};

/* HEARTBEAT_MONITOR */
struct bnd_msg_heartbeat {
	CHAR ucIfName[32];
};

struct bnd_msg_cli_probe {
	BOOLEAN bAllowStaConnectInHt;
	BOOLEAN bIosCapable;   /* For IOS immediately connect */
	UINT8	bVHTCapable;
	UINT8	Nss;
	CHAR	Rssi[4];
};

struct bnd_msg_cli_auth {
	CHAR	Rssi[4];
};

struct bnd_msg_cli_assoc {
	BOOLEAN bAllowStaConnectInHt;
	UINT8	bVHTCapable;
	UINT8	Nss;
	UINT8	BTMSupport;
/* WPS_BandSteering Support */
	BOOLEAN bWpsAssoc;
};

struct bnd_msg_cli_delete {
};

/* CLI_EVENT */
struct bnd_msg_cli_event {
	UINT8	FrameType;
	UINT8	Band;
	UINT8   Channel;
	UCHAR	Addr[MAC_ADDR_LEN];
	union {
		struct bnd_msg_cli_probe cli_probe;
		struct bnd_msg_cli_auth cli_auth;
		struct bnd_msg_cli_assoc cli_assoc;
		struct bnd_msg_cli_delete cli_delete;
	} data;
};

/* CLI_ADD */
struct bnd_msg_cli_add {
	UINT8 TableIndex;
	UCHAR Addr[MAC_ADDR_LEN];
};

/* CLI_DEL */
struct bnd_msg_cli_del {
	UINT8 TableIndex;
	UCHAR Addr[MAC_ADDR_LEN];
};

/* CLI_STATUS_REQ */
struct bnd_msg_cli_status_req {
};

/* CLI_STATUS_RSP */
struct bnd_msg_cli_status_rsp {
	UINT8 TableIndex;
	UINT8 ReturnCode;
	UCHAR	Addr[MAC_ADDR_LEN];
	char	data_Rssi;
	UINT32	data_tx_Rate;
	UINT32	data_rx_Rate;
	UINT64	data_tx_Byte;
	UINT64	data_rx_Byte;
	UINT8	data_tx_Phymode;
	UINT8	data_rx_Phymode;
	UINT8	data_tx_mcs;
	UINT8	data_rx_mcs;
	UINT8	data_tx_bw;
	UINT8	data_rx_bw;
	UINT8	data_tx_sgi;
	UINT8	data_rx_sgi;
	UINT8	data_tx_stbc;
	UINT8	data_rx_stbc;
	UINT8	data_tx_ant;
	UINT8	data_rx_ant;
	UINT64	data_tx_packets;
	UINT64	data_rx_packets;
};

/* CHANLOAD_STATUS_REQ */
struct bnd_msg_chanload_status_req {
	CHAR ucIfName[32];
	/* u8 band; */
	/* u8 Channel; */
};
/* CHANLOAD_STATUS_RSP */
struct bnd_msg_chanload_status_rsp {
	UINT8 ReturnCode;
	UINT8 band;
	UINT8 Channel;
	/* TBD */
	UINT8	chanload;
	UINT8	chan_busy_load;
	UINT8	obss_load;
	UINT8	edcca_load;
	UINT8	myair_load;
	UINT8	mytxair_load;
	UINT8	myrxair_load;
};

/* INF_STATUS_QUERY */
struct bnd_msg_inf_status_req {
    CHAR ucIfName[32];
};

struct bnd_msg_inf_status_rsp {
    BOOLEAN bInfReady;
    UINT8 Channel;
    BOOLEAN bVHTCapable;
    ULONG table_src_addr;
    CHAR ucIfName[32];
	UINT8 nvram_support;
	UINT8 nss;
	UINT8 band;
	UINT32 table_size;
};

/* BNDSTRG_ONOFF */
struct bnd_msg_onoff {
    UINT8 Band;
    UINT8 Channel;
    BOOLEAN OnOff;
	UINT8	BndStrgMode;
    CHAR ucIfName[32];
};

/* set band steering paramater */
struct bnd_msg_param {
	UINT8 Band;
	UINT8 Channel;
	UINT8 len;
	CHAR arg[64];
};

struct bnd_msg_neighbor_report {
	UCHAR Addr[MAC_ADDR_LEN];
	UINT8 Band;
	UINT8 Channel;
	CHAR  NeighborRepInfo[64];
};

/* SET_MNT_ADDR */
struct bnd_msg_mnt_addr {
	UCHAR Addr[MAC_ADDR_LEN];
};

/* NVRAM_UPDATE */
struct bnd_msg_nvram_entry_update {
    UCHAR Addr[MAC_ADDR_LEN];
	BNDSTRG_NVRAM_CLIENT nvram_entry;
};

struct bnd_msg_reject_body {
    UINT32 DaemonPid;
};

/*display type for list*/
struct bnd_msg_display_entry_list {
    UINT32 display_type;
	UINT8 filer_band;
	UINT8 channel;
};

struct bnd_msg_update_white_black_list {
    CHAR ucIfName[32];
	UINT8 list_type;
	UCHAR Addr[MAC_ADDR_LEN];
	BOOLEAN	deladd;
};

struct bnd_msg_wnm_command {
	UINT8 wnm_data[64];
};

typedef struct _BNDSTRG_MSG {
	UINT8   Action;
	union {
		struct bnd_msg_cli_event cli_event;
		struct bnd_msg_cli_add cli_add;
		struct bnd_msg_cli_del cli_del;
		struct bnd_msg_cli_status_req cli_status_req;
		struct bnd_msg_cli_status_rsp cli_status_rsp;
		struct bnd_msg_chanload_status_req chanload_status_req;
		struct bnd_msg_chanload_status_rsp chanload_status_rsp;
		struct bnd_msg_inf_status_req inf_status_req;
		struct bnd_msg_inf_status_rsp inf_status_rsp;
		struct bnd_msg_onoff onoff;
		struct bnd_msg_mnt_addr mnt_addr;
		struct bnd_msg_nvram_entry_update entry_update;
		struct bnd_msg_reject_body reject_body;
		struct bnd_msg_display_entry_list display_type;
		struct bnd_msg_heartbeat heartbeat;
		struct bnd_msg_param bndstrg_param;
		struct bnd_msg_update_white_black_list	update_white_black_list;
		struct bnd_msg_wnm_command wnm_cmd_data;
		struct bnd_msg_neighbor_report Neighbor_Report;
	} data;
} BNDSTRG_MSG, *PBNDSTRG_MSG;
#endif /* BAND_STEERING */
#endif /* _BAND_STEERING_DEF_H_ */

