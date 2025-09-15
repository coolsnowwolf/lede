#ifdef WH_EVENT_NOTIFIER
#ifndef __EVENT_NOTIFIER_H__
#define __EVENT_NOTIFIER_H__

/* Wireless Capability Flag */
#define WHC_WLCAP_80211_N               (0x00000001)
#define WHC_WLCAP_80211_AC              (0x00000002)
#define WHC_WLCAP_RESERVE_1             (0x00000004)
#define WHC_WLCAP_RESERVE_2             (0x00000008)
#define WHC_WLCAP_HT40                  (0x00000010)
#define WHC_WLCAP_HT80                  (0x00000020)
#define WHC_WLCAP_HT160                 (0x00000040)
#define WHC_WLCAP_RESERVE_3             (0x00000080)
#define WHC_WLCAP_RX_2_STREAMS          (0x00000100)
#define WHC_WLCAP_RX_3_STREAMS          (0x00000200)
#define WHC_WLCAP_RX_4_STREAMS          (0x00000400)
#define WHC_WLCAP_TX_2_STREAMS          (0x00000800)
#define WHC_WLCAP_TX_3_STREAMS          (0x00001000)
#define WHC_WLCAP_TX_4_STREAMS          (0x00002000)
#define WHC_WLCAP_RESERVE_4             (0x00004000)
#define WHC_WLCAP_RESERVE_5             (0x00008000)

#define WHC_UPLINK_STAT_DISCONNECT      (0)
#define WHC_UPLINK_STAT_CONNECTED       (1)

enum {
    DRVEVNT_FIRST_ID = 0,
    WHC_DRVEVNT_STA_PROBE_REQ,
    WHC_DRVEVNT_AP_PROBE_RSP,
    WHC_DRVEVNT_STA_JOIN,
    WHC_DRVEVNT_STA_LEAVE,
    WHC_DRVEVNT_EXT_UPLINK_STAT,
    WHC_DRVEVNT_STA_TIMEOUT,
    WHC_DRVEVNT_STA_AUTH_REJECT,
    WHC_DRVEVNT_CHANNEL_LOAD_REPORT,
    WHC_DRVEVNT_STA_RSSI_TOO_LOW,
    WHC_DRVEVNT_STA_ACTIVITY_STATE,
    DRVEVNT_END_ID,
};

enum {
    WHC_STA_STATE_IDLE = 0,
    WHC_STA_STATE_ACTIVE,
};

struct Custom_IE_Header {
    UCHAR eid;
    UINT8 len;
};

struct Custom_VIE {
    struct Custom_IE_Header ie_hdr;
    UCHAR custom_ie[CUSTOM_IE_TOTAL_LEN];
};

typedef NTSTATUS (*EventHdlr)(IN RTMP_ADAPTER *pAd, ...);

typedef struct {
	UINT32 EventID;
	EventHdlr EventHandler;
}EVENT_TABLE_T;
typedef struct _RxInfo{
    UCHAR channel;         				    /* Wireless channel that receive this frame. */
	ULONG rate;                             /* RX data rate when recv frame */
    CHAR  rssi;                     		/* RSSI when recv frame */
}RxInfo, *PRxInfo;

typedef struct _ChLoadItem {
    BOOLEAN bValid;                         /* Check if it is an valid reference info */
    UCHAR Channel;                          /* Which channel */
    UINT32 ChannelBusyTime;                 /* Channel busy Time */
}ChLoadItem, *PChLoadItem;

typedef struct _StaActivityItem {
    ULONG PacketCount;                      /* Packet count */
    UCHAR CurrentState;                     /* Station's current state */
}StaActivityItem, *PStaActivityItem;
struct drvevnt_sta_probe_req {
    USHORT type;            				/* WHC_DRVEVNT_STA_PROBE_REQ */
    UCHAR sta_mac[MAC_ADDR_LEN];    		/* Station's MAC address */
    BOOLEAN is_ucast;        				/* If unicast frame */
    UINT32 cap;              				/* Refer to Wireless Capability Flag */
    RxInfo rx_info;
    UCHAR custom_ie_len;        			/* Length of custom vendor information element */
    UCHAR custom_ie[CUSTOM_IE_TOTAL_LEN];   /* Content of custom vendor information element */
};

struct drvevnt_ap_probe_rsp {
    USHORT type;            				/* WHC_DRVEVNT_AP_PROBE_RSP */
    UCHAR ap_mac[MAC_ADDR_LEN];    		    /* AP's MAC address */
    UINT32 cap;              				/* Refer to Wireless Capability Flag */
    RxInfo rx_info;
    UCHAR custom_ie_len;        			/* Length of custom vendor information element */
    UCHAR custom_ie[CUSTOM_IE_TOTAL_LEN];   /* Content of custom vendor information element */
};

struct drvevnt_sta_join {
    USHORT type;            				/* WHC_DRVEVNT_STA_JOIN */
    UCHAR sta_mac[MAC_ADDR_LEN];    		/* Station's MAC address */
    UCHAR channel;         				    /* Wireless channel that receive this frame. */
    UINT32 aid;               				/* Station's association ID */
    UCHAR custom_ie_len;        			/* Length of custom vendor information element */
    UCHAR custom_ie[CUSTOM_IE_TOTAL_LEN];   /* Content of custom vendor information element */
    UINT32 capability;	
};

struct drvevnt_sta_leave {
    USHORT type;            				/* WHC_DRVEVNT_STA_LEAVE */
    UCHAR sta_mac[MAC_ADDR_LEN];    		/* Station's MAC address */
    UCHAR channel;         				    /* Wireless channel that receive this frame. */
};

struct drvevnt_ext_uplink_stat {
    USHORT type;            				/* WHC_DRVEVNT_EXT_UPLINK_STAT */
    UCHAR ap_mac[MAC_ADDR_LEN];    		    /* AP's MAC address */
    UCHAR channel;         				    /* Wireless channel that receive this frame. */
    UINT32 link_state;               		/* Station's  link status*/
};

struct drvevnt_sta_timeout {
    USHORT type;            				/* WHC_DRVEVNT_STA_LEAVE */
    UCHAR sta_mac[MAC_ADDR_LEN];    		/* Station's MAC address */
    UCHAR channel;         				    /* Current Wireless channel. */
};

struct drvevnt_sta_auth_reject {
    USHORT type;            				/* WHC_DRVEVNT_STA_AUTH_REJECT */
    UCHAR sta_mac[MAC_ADDR_LEN];    		/* Station's MAC address */
    RxInfo rx_info;
};

struct drvevnt_channel_load_report {
    USHORT type;            			   /* WHC_DRVEVNT_CHANNEL_LOAD_REPORT */
    ChLoadItem ch_load_2g;    		       /* Channel Load Item for 2GHz */
    ChLoadItem ch_load_5g;                 /* Channel Load Item for 5GHz */
};

struct drvevnt_sta_rssi_too_low {
    USHORT type;            				/* WHC_DRVEVNT_STA_RSSI_TOO_LOW */
    UCHAR sta_mac[MAC_ADDR_LEN];    		/* Station's MAC address */
    CHAR rssi;                              /* Station's Rssi */
};

struct drvevnt_sta_activity_stat {
    USHORT type;            				/* WHC_DRVEVNT_STA_ACTIVITY_STATE */
    UCHAR sta_mac[MAC_ADDR_LEN];    		/* Station's MAC address */
    BOOLEAN bTx;                            /* Check if it is for tx record */
    StaActivityItem sta_state;              /* Station's state record */
};

struct EventNotifierCfg {
    UCHAR CustomOUILen;
    UCHAR CustomOUI[8];

    /* For WHC_DRVEVNT_CHANNEL_LOAD_REPORT */
    BOOLEAN bChLoadDetect;
    UINT32 ChLoadRound;
    UINT32 ChLoadDetectPeriod; /* unit: second */
    struct drvevnt_channel_load_report Chload;

    /* For WHC_DRVEVNT_STA_RSSI_TOO_LOW */
    BOOLEAN bStaRssiDetect;
    CHAR StaRssiDetectThreshold;

    /* WHC_DRVEVNT_STA_ACTIVITY_STATE */
    UINT32 StaTxPktDetectRound;     /* Record current Tx round, per second */
    UINT32 StaTxPktDetectPeriod;    /* unit: second */
    BOOLEAN bStaStateTxDetect;
    UINT32 StaStateTxThreshold;
    UINT32 StaRxPktDetectRound;     /* Record current Tx round, per second */
    UINT32 StaRxPktDetectPeriod;    /* unit: second */
    BOOLEAN bStaStateRxDetect;
    UINT32 StaStateRxThreshold;
};

struct drvevnt_type_buf {
    union {
        struct drvevnt_sta_probe_req   probereq;
        struct drvevnt_ap_probe_rsp    probersp;
        struct drvevnt_sta_join        join;
        struct drvevnt_sta_leave       leave;
        struct drvevnt_ext_uplink_stat link_state;
        struct drvevnt_sta_timeout     timeout;
        struct drvevnt_sta_auth_reject auth_reject;
        struct drvevnt_sta_rssi_too_low sta_rssi;
        struct drvevnt_sta_activity_stat sta_activity_stat;
    } data;
};

EventHdlr GetEventNotiferHook(IN UINT32 EventID);
INT SetCustomOUIProc(PRTMP_ADAPTER pAd, PSTRING arg);
INT SetCustomVIEProc(PRTMP_ADAPTER pAd, PSTRING arg);
INT SetChannelLoadDetectPeriodProc(PRTMP_ADAPTER pAd, PSTRING arg);
INT SetStaRssiDetectThresholdProc(PRTMP_ADAPTER pAd, PSTRING arg);
INT SetStaTxPktDetectPeriodProc(PRTMP_ADAPTER pAd, PSTRING arg);
INT SetStaTxPacketDetectThresholdProc(PRTMP_ADAPTER pAd, PSTRING arg);
INT SetStaRxPktDetectPeriodProc(PRTMP_ADAPTER pAd, PSTRING arg);
INT SetStaRxPacketDetectThresholdProc(PRTMP_ADAPTER pAd, PSTRING arg);
VOID WHCMlmePeriodicExec(PRTMP_ADAPTER pAd);

#endif /* __EVENT_NOTIFIER_H__ */
#endif
