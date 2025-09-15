#ifndef __EVENT_NOTIFIER_H__
#define __EVENT_NOTIFIER_H__

#define CUSTOM_IE_TOTAL_LEN 128

enum {
    DRVEVNT_FIRST_ID = 0,
    WHC_DRVEVNT_STA_JOIN = 3,
    WHC_DRVEVNT_STA_LEAVE,
    DRVEVNT_END_ID,
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

#endif /* __EVENT_NOTIFIER_H__ */
