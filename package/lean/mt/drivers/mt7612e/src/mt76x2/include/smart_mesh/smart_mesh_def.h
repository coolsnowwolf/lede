#ifndef __SMART_MESH_DEF_H__
#define __SMART_MESH_DEF_H__

/*
NTGR VIE--->
The 1st byte contains the following information:
        Bit  Description
        0	 WLAN driver support DWDS
        1	 System support SmartMesh
        2	 System supports Orbi
        3-4 Used by Orbi daemon
        5	 Orbi WPS capability
        6-7	 reserved

The 3rd byte contains the following information:
        Bit  Description
        0	 Orbi WPS trigger status
        1-7	 reserved
*/

#define NTGR_IE_TOTAL_LEN 160
#define NTGR_OUI_LEN 3
#define NTGR_CUSTOM_IE_MAX_LEN 3
#define NTGR_IE_PRIV_LEN (NTGR_IE_TOTAL_LEN - NTGR_OUI_LEN)
extern UCHAR NETGEAR_OUI[NTGR_OUI_LEN];

#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
enum hidde_wps_state_type {
    HIDDEN_WPS_STATE_STOP = 0,
    HIDDEN_WPS_STATE_RUNNING = (1<<0)
};

enum hidde_wps_role_type {
    HIDDEN_WPS_ROLE_ENROLLEE = 0,
    HIDDEN_WPS_ROLE_REGISTRAR = (1<<1)
};
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */

#ifdef SMART_MESH_MONITOR
#define NSMP_MAX_HT_CHANNELS        8
#define MAX_NUM_OF_MONITOR_STA		7
#define WCID_OF_MONITOR_STA_BASE	(MAX_NUMBER_OF_MAC - MAX_NUM_OF_MONITOR_STA)
#define MAX_WCID_OF_MONITOR_STA		(MAX_NUMBER_OF_MAC - 1)

/* WLAN capability information */
#define NSMP_WLCAP_80211_N              (0x00000001)
#define NSMP_WLCAP_80211_AC             (0x00000002)
#define NSMP_WLCAP_RESERVE_1            (0x00000004)
#define NSMP_WLCAP_RESERVE_2            (0x00000008)
#define NSMP_WLCAP_HT40                 (0x00000010)
#define NSMP_WLCAP_HT80                 (0x00000020)
#define NSMP_WLCAP_HT160                (0x00000040)
#define NSMP_WLCAP_RESERVE_3            (0x00000080)
#define NSMP_WLCAP_RX_2_STREAMS         (0x00000100)
#define NSMP_WLCAP_RX_3_STREAMS         (0x00000200)
#define NSMP_WLCAP_RX_4_STREAMS         (0x00000400)
#define NSMP_WLCAP_TX_2_STREAMS         (0x00000800)
#define NSMP_WLCAP_TX_3_STREAMS         (0x00001000)
#define NSMP_WLCAP_TX_4_STREAMS         (0x00002000)
#define NSMP_WLCAP_RESERVE_4            (0x00004000)
#define NSMP_WLCAP_RESERVE_5            (0x00008000)

#define NSMP_UPLINK_STAT_DISCONNECT     (0)
#define NSMP_UPLINK_STAT_CONNECTED      (1)

#define NSMP_WPS_PBC_STAT_BEGIN         (0)
#define NSMP_WPS_PBC_STAT_END_SUCCESS   (1)
#define NSMP_WPS_PBC_STAT_END_TIMEOUT   (2)


enum nsmpif_driver_event_type {
    NSMPIF_DRVEVNT_RESPONSE = 0,
    NSMPIF_DRVEVNT_STA_PROBE_REQ,   /* Driver receive a ProbeReq */
    NSMPIF_DRVEVNT_STA_AUTH_REQ,    /* Not used currently */
    NSMPIF_DRVEVNT_STA_ASSOC_REQ,   /* Not used currently */
    NSMPIF_DRVEVNT_STA_JOIN,        /* Once a station is AUTHORISED. If WPA used, it need to finish 4 ways handshake; otherwise when station complete ASSOC Req/Resp */
    NSMPIF_DRVEVNT_STA_LEAVE,       /* Receive or Send DEAUTH to a station */
    NSMPIF_DRVEVNT_STA_TIMEOUT,     /* Station leave due to inactivity timeout */
    /* The event is optional, if driver is using LEAVE event for timeout client */
    NSMPIF_DRVEVNT_EXT_UPLINK_STAT, /* For Extender only, triggered when link to Root-AP established or dropped */
    NSMPIF_DRVEVNT_WPS_PBC_STAT,    /* Triggered when device WPS PBC for AP interface is started or end */
    NSMPIF_DRVEVNT_STA_AUTH_REJECT, /* Triggered when a STA rejected due to ACL rule */
    NSMPIF_DRVEVNT_CHANNEL_CHANGE,  /* Triggered when a AP interface changes channel, such as ACSD */
    NSMPIF_DRVEVNT_STA_WPA_KEYERR,  /* Triggered when a station using incorrect password during WPA handshake */
    NSMPIF_DRVEVNT_AP_PROBE_RESP,  /* Driver receive a ProbeResp */
    NSMPIF_DRVEVNT_MAX,
};
/* Note: Smart Mesh daemon will use "struct nsmpif_drvevnt_response" to reponse all event types */

struct nsmpif_drvevnt_sta_probe_req {
    unsigned short type;            				/* NSMPIF_DRVEVNT_STA_PROBE_REQ */
    unsigned short channel;         				/* Wireless channel that receive probe request frame. */
    unsigned char sta_mac[MAC_ADDR_LEN];    		/* Station's MAC address */
    unsigned short is_ucast;        				/* if unicast frame */
    unsigned int cap;              					/* refer to NSMP_WLCAP_XXX in nsmp_common.h */
    int rate;                       /* RX data rate when recv frame, Kbps */
    short rssi;                     				/* RSSI when recv frame */
    short snr;                      				/* SNR when recv frame */
	unsigned short ntgr_vie_len;        			/* Length of NETGEAR Vendor Information Element */
    char ntgr_vie[NTGR_IE_TOTAL_LEN];    			/* NTGR Vendor Information Element without OUI */
};

struct nsmpif_drvevnt_sta_auth_req {
    unsigned short type;            				/* NSMPIF_DRVEVNT_STA_AUTH_REQ */
    unsigned short channel;         				/* Wireless channel, main channel only */
    unsigned char sta_mac[MAC_ADDR_LEN];    		/* Station's MAC address */
    unsigned short reserved_1;
    unsigned int reserved_2;
    int rate;                       /* RX data rate when recv frame, Kbps */
    short rssi;                     				/* RSSI when recv frame */
    short snr;                     					/* SNR when recv frame */
    unsigned short ntgr_vie_len;                    /* Length of NETGEAR Vendor Information Element */
    unsigned char ntgr_vie[NTGR_IE_TOTAL_LEN];   /* NTGR Vendor Information Element without OUI */
};

struct nsmpif_drvevnt_sta_assoc_req {
    unsigned short type;            				/* NSMPIF_DRVEVNT_STA_ASSOC_REQ */
    unsigned short channel;         				/* Wireless channel, main channel only */
    unsigned char sta_mac[MAC_ADDR_LEN];    		/* Station's MAC address */
    unsigned int cap;               				/* Refer to NSMP_WLCAP_XXX in nsmp_common.h */
    unsigned short ntgr_vie_len;                    /* Length of NETGEAR Vendor Information Element */
    unsigned char ntgr_vie[NTGR_IE_TOTAL_LEN];   /* NTGR Vendor Information Element without OUI */
};

struct nsmpif_drvevnt_sta_join {
    unsigned short type;            				/* NSMPIF_DRVEVNT_STA_JOIN */
    unsigned short channel;         				/* Wireless channel, main channel only */
    unsigned char sta_mac[MAC_ADDR_LEN];    		/* Station's MAC address */
    unsigned int aid;               				/* Station's association ID */
};

struct nsmpif_drvevnt_sta_leave {
    unsigned short type;            				/* NSMPIF_DRVEVNT_STA_LEAVE */
    unsigned short channel;         				/* Wireless channel, main channel only */
    unsigned char sta_mac[MAC_ADDR_LEN];    		/* Station's MAC address */
};

struct nsmpif_drvevnt_sta_timeout {
    unsigned short type;            				/* NSMPIF_DRVEVNT_STA_TIMEOUT */
    unsigned short channel;         				/* Wireless channel, main channel only */
    unsigned char sta_mac[MAC_ADDR_LEN];    		/* Station's MAC address */
};

struct nsmpif_drvevnt_ext_uplink_stat {
    unsigned short type;                                /* NSMPIF_DRVEVNT_EXT_UPLINK_STAT */
    unsigned short channel;                             /* Wireless channel, main channel only */
    unsigned short op_channels[NSMP_MAX_HT_CHANNELS];   /*
														 * operating channels up to HT160 (8 channels)
														 * channel[0] to channel[7]  (set to zero if not used)
														 * Ex: If radio is HT40 44(P) + 48(S)
														 * Set channel as:
														 *     channel[0] = 44;
														 *     channel[1] = 48;
														 *     channel[2] = 0;
														 *     ...
														 *     channel[7] = 0;
														 */
    unsigned short link_state;                          /* NSMP_UPLINK_STAT_DISCONNECT or other defined in nsmp_common.h */
    unsigned char bssid[6];                             /* RootAP's BSSID when event is NSMP_UPLINK_STAT_CONNECTED, else NULL */
};

struct nsmpif_drvevnt_wps_pbc_stat {
    unsigned short type;            				/* NSMPIF_DRVEVNT_WPS_PBC_STAT */
    unsigned short stat;            				/* NSMP_WPS_PBC_STAT_BEGIN or others definied in nsmp_common.h */
    unsigned short reason;							/* Success:0, Fail: none-zero */
    unsigned short channel;							/* Wireless channel, current channel only */
    unsigned char sta_mac[MAC_ADDR_LEN];    		/* Station's MAC if stat is NSMP_PBC_STAT_END_SUCCESS, else NULL */
    unsigned short reserve;
};

struct nsmpif_drvevnt_sta_auth_reject_req {
    unsigned short type;            				/* NSMPIF_DRVEVNT_STA_AUTH_REJECT */
    unsigned short channel;         				/* Wireless channel, main channel only */
    unsigned char sta_mac[MAC_ADDR_LEN];    		/* Station's MAC address */
    unsigned short is_ucast;        /* Not used. For aligning struct nsmpif_drvevnt_sta_probe_req */
    unsigned int cap;               /* Not used. For aligning struct nsmpif_drvevnt_sta_probe_req */
    int rate;                       /* RX data rate when recv frame, Kbps */
    short rssi;                     				/* RSSI when recv frame */
    short snr;                      				/* SNR when recv frame */
    unsigned short ntgr_vie_len;                    /* Length of NETGEAR Vendor Information Element */
    unsigned char ntgr_vie[NTGR_IE_TOTAL_LEN];   /* NTGR Vendor Information Element without OUI */
};

struct nsmpif_drvevnt_channel_change_req {
    unsigned short type;            					/* NSMPIF_DRVEVNT_CHANNEL_CHANGE */
    unsigned short channel;         					/* Wireless channel, main channel only */
    unsigned short op_channels[NSMP_MAX_HT_CHANNELS];   /*
														 * operating channels up to HT160 (8 channels)
														 * channel[0] to channel[7]  (set to zero if not used)
														 * Ex: If radio is HT40 44(P) + 48(S)
														 * Set channel as:
														 *     channel[0] = 44;
														 *     channel[1] = 48;
														 *     channel[2] = 0;
														 *     ...
														 *     channel[7] = 0;
														 */
};

struct nsmpif_drvevnt_sta_wpa_keyerr {
    unsigned short type;            				/* NSMPIF_DRVEVNT_STA_WPA_KEYERR */
    unsigned short channel;         				/* Wireless channel, main channel only */
    unsigned char sta_mac[MAC_ADDR_LEN];   			/* Station's MAC address */
    unsigned short reserve;
};

struct nsmpif_drvevnt_ap_probe_resp {
    unsigned short type;                        /* NSMPIF_DRVEVNT_STA_PROBE_RSP */
    unsigned short channel;                     /* Wireless channel that receive probe response frame. */
    unsigned char ap_mac[MAC_ADDR_LEN]; 		/* AP's MAC address */
    unsigned short is_ucast;                    /* if unicast frame */
    unsigned int cap;                           /* refer to NSMP_WLCAP_XXX in nsmp_common.h */
    int rate;                                   /* RX data rate when recv frame */
    short rssi;                                 /* RSSI when recv frame */
    short snr;                                  /* SNR when recv frame */
    char ssid[MAX_LEN_OF_SSID];           		/* SSID in Probe Resp, non-used bytes must be zero */
    unsigned short ntgr_vie_len;                /* Length of NTGR Vendor Information Element */
    char ntgr_vie[NTGR_IE_TOTAL_LEN];        	/* NTGR Vendor Information Element without OUI */
};

struct nsmpif_drvevnt_response {    
	int response;                   /* NSMP_RETURN_SUCCESS or NSMP_RETURN_FAILURE defined in nsmp_common.h */
};

/* used to cast buffer for examining message type */
struct nsmpif_drvevnt_hdr {
    unsigned short type;
    unsigned short channel;         /* Wireless channel, main channel only */
};

/* a larger buffer able to handle all kind of messages */
struct nsmpif_drvevnt_buf {
    union {
        struct nsmpif_drvevnt_sta_probe_req         probereq;
        struct nsmpif_drvevnt_sta_auth_req          authreq;
        struct nsmpif_drvevnt_sta_assoc_req         assocreq;
        struct nsmpif_drvevnt_sta_join              join;
        struct nsmpif_drvevnt_sta_leave             leave;
        struct nsmpif_drvevnt_sta_timeout           timeout;
        struct nsmpif_drvevnt_ext_uplink_stat       linkstate;
        struct nsmpif_drvevnt_response              response;
        struct nsmpif_drvevnt_wps_pbc_stat          wps_pbc_stat;
        struct nsmpif_drvevnt_sta_auth_reject_req   authreject;
        struct nsmpif_drvevnt_channel_change_req    channel_change;
        struct nsmpif_drvevnt_sta_wpa_keyerr        sta_wpa_keyerr;
        struct nsmpif_drvevnt_ap_probe_resp         proberesp;
    } data;
};

typedef struct GNU_PACKED _HEADER_802_11_4_ADDR {
    FRAME_CONTROL   FC;
    USHORT          Duration;
    UCHAR           Addr1[MAC_ADDR_LEN];
    UCHAR           Addr2[MAC_ADDR_LEN];
	UCHAR			Addr3[MAC_ADDR_LEN];
#ifdef RT_BIG_ENDIAN
	USHORT			Sequence:12;
	USHORT			Frag:4;
#else
	USHORT			Frag:4;
	USHORT			Sequence:12;
#endif /* !RT_BIG_ENDIAN */
	UCHAR			Addr4[MAC_ADDR_LEN];
}	HEADER_802_11_4_ADDR, *PHEADER_802_11_4_ADDR;

typedef struct GNU_PACKED _SMART_RADIO_INFO{
	CHAR RSSI0;
	CHAR RSSI1;	
	CHAR SNR0;
	CHAR SNR1;
	CHAR PHYMODE;
	CHAR MCS;
	CHAR BW;
	CHAR ShortGI;
	UINT32 RATE;
} SMART_RADIO_INFO, *PSMART_RADIO_INFO;

typedef struct GNU_PACKED _SMART_MESH_RAW {
	 SMART_RADIO_INFO wlan_radio_tap;
	 HEADER_802_11_4_ADDR wlan_header;
} SMART_MESH_RAW, *PSMART_MESH_RAW;

#endif /*SMART_MESH_MONITOR*/

#ifdef SMART_MESH
#define MAX_TX_RX_STREAMS 3
#define MAX_LAST_PKT_STATS 20
#define ETH_TYPE_SMART_MESH 0x99AA

enum SMART_MESH_IE_CTRL_FLAG {
	SM_IE_BEACON = (1<<0),
	SM_IE_PROBE_RSP = (1<<1),
	SM_IE_ASSOC_RSP = (1<<2),
	SM_IE_AUTH_RSP  = (1<<3),
	SM_IE_PROBE_REQ  = (1<<4),
	SM_IE_AUTH_REQ  = (1<<5),
	SM_IE_ASSOC_REQ = (1<<6),
	SM_IE_ALL = 0xFFFF
};


/*
Character 	Bit 		Information Provided
	0 		0 		value 1: DWDS is enabled,
					value 0: DWDS is disabled
------------------------------------------
	0 		1 		value 1: SmartMesh is
					enabled, value 0: SmartMesh
					is disabled
------------------------------------------					
x x Others are Reserved
------------------------------------------
*/
typedef struct _NTGR_IE {
	UINT8	id; /* 221 */
	UINT8	len;
	UINT8	oui[NTGR_OUI_LEN]; /* 0x00 0x14 0x6C Netgear OUI */
	UINT8	private[NTGR_IE_PRIV_LEN];
}NTGR_IE, *PNTGR_IE;

typedef struct _UNI_PROBE_REQ_CFG {
	UINT8 		ssid_len;
	UCHAR		addr1[MAC_ADDR_LEN];
	UCHAR		addr2[MAC_ADDR_LEN];
	CHAR 		ssid[MAX_LEN_OF_SSID];
}UNI_PROBE_REQ_CFG, *PUNI_PROBE_REQ_CFG;

typedef struct _SMART_MESH_CFG {
	UINT32		ie_ctrl_flags;
	BOOLEAN		bSupportSmartMesh; 	/* Determine If own smart mesh capability */
#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
    BOOLEAN		bSupportHiddenWPS; 	/* Determine If own Hidden WPS capability */
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */
	BOOLEAN		bHiFiPeerFilter; 	/* Only allow Hi-Fi target device to negotiate */
	CHAR		HiFiFlagMask; 		/* A mask for identifying Hi-Fi bit in vendor IE */
	UCHAR		HiFiFlagValue; 		/* A desired match value for identifying Hi-Fi device in vendor IE */
	NTGR_IE		Ntgr_IE;
	UNI_PROBE_REQ_CFG UniProbeReqCfg;
}SMART_MESH_CFG, *PSMART_MESH_CFG;

typedef struct _SMART_MESH_CLI_INFO {
	USHORT aid;
	CHAR rssi;
	CHAR snr;

	/* RX Info */
	UINT8   rx_mcs;
	UINT8   rx_stream;
	UINT8   rx_bw;
	UINT8   rx_sgi;
	UINT8   rx_phymode;
	CHAR	last_rssi[MAX_TX_RX_STREAMS];
	CHAR	last_snr[MAX_TX_RX_STREAMS];

	/* TX Info */
	HTTRANSMIT_SETTING TX_HTSetting;
	UINT8   tx_mcs;
	UINT8   tx_stream;
	UINT8   tx_bw;
	UINT8   tx_sgi;
	UINT8   tx_phymode;
	
	/* statistics */
	INT64	rx_total_packets;
	ULONG	rx_total_bytes;
	ULONG	rx_fail_cnt;
	INT64	tx_total_packets;
	ULONG	tx_total_bytes;
	ULONG	tx_fail_cnt;
} SMART_MESH_CLI_INFO, *PSMART_MESH_CLI_INFO;

/* For  RT_OID_SET_LAST_TX_RX_STATS */
typedef struct _SET_PKT_STATS_INFO {
    BOOLEAN bCliPktStatEnable;
    UCHAR   Mac[MAC_ADDR_LEN];
} SET_PKT_STATS_INFO, *PSET_PKT_STATS_INFO;

/* For  RT_OID_GET_LAST_TX_RX_STATS */
typedef struct _CLINET_PKT_STATS_INFO {
    BOOLEAN bTxValid;
    BOOLEAN bRxValid;
    
	/* RX Info */
	CHAR	last_rssi[MAX_TX_RX_STREAMS];
    UINT32  rx_seq;
    UINT8   rx_mcs;
    UINT8   rx_stream;
	UINT8   rx_bw;
	UINT8   rx_sgi;
	UINT8   rx_phymode;
    UINT32  rx_rate; 
    ULONG   rx_time_sec;

	/* TX Info */
    UINT32  tx_seq;
    UINT8   tx_mcs;
	UINT8   tx_stream;
	UINT8   tx_bw;
	UINT8   tx_sgi;
	UINT8   tx_phymode;
    UINT32  tx_rate; 
    ULONG   tx_time_sec;
} CLINET_PKT_STATS_INFO, *PCLINET_PKT_STATS_INFO;

/* For  RT_OID_SET_PKT_TX_RX_STATS */
typedef struct _SET_PKT_TX_RX_STATS {
#ifdef ED_MONITOR
    BOOLEAN timer_en;
#endif /* ED_MONITOR */	
	ULONG scan_time; /* unit: msec */
	BOOLEAN trigger_site_survey;
	BOOLEAN channel_time_init;
    //UCHAR   Mac[MAC_ADDR_LEN];
} SET_PKT_TX_RX_STATS, *PSET_PKT_TX_RX_STATS;

typedef enum FalseCCASET {
	CCA_RESET,
	CCA_STORE
} False_CCA_SET;

typedef struct _CHANNEL_INFO {
		UCHAR channel;
		ULONG false_cca;
}CHANNEL_INFO, *PCHANNEL_INFO;

typedef struct _CLIENT_PKT_INFO{
	UCHAR	mac[MAC_ADDR_LEN];
	USHORT	aid;
	ULONG	tx_per;
	ULONG 	tx_acked_packet;
	ULONG 	agg_size;
} CLIENT_PKT_INFO, *PCLIENT_PKT_INFO;

typedef struct _PKT_TX_RX_INFO {
	ULONG  			tx_acked_num;
	ULONG 			rx_pkt_error_rate;
	ULONG			false_cca_count;
	ULONG			edcca_busy_time;
	ULONG  			rts_success_count;
	ULONG  			rts_fail_count;
	ULONG 			channel_busy_time;
	ULONG 			channel_idle_time;
	UCHAR 			channel_list_num;
	CHANNEL_INFO	channel_list[MAX_NUM_OF_CHANNELS];
	USHORT	entry_num;
	CLIENT_PKT_INFO entry[MAX_NUMBER_OF_MAC];

} PKT_TX_RX_INFO, *PPKT_TX_RX_INFO;

typedef struct _AP_INFO{
	UCHAR 	ssid[MAX_LEN_OF_SSID];
	UCHAR 	ssid_len;
	USHORT  channel;
	CHAR 	bw;
	CHAR	rssi;
} AP_INFO, *PAP_INFO;

typedef struct _AP_LIST_INFO {
	UCHAR num;	
	UCHAR channel_ap_num;
	USHORT channel;
	AP_INFO ap[MAX_LEN_OF_BSS_TABLE];
} AP_LIST_INFO, *PAP_LIST_INFO;

#endif /* SMART_MESH */
#endif /* __SMART_MESH_DEF_H__ */

