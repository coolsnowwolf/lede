#ifndef __ALL_NET_EVENT__
#define __ALL_NET_EVENT__

typedef enum FBT_NOTIFY_TYPE {
	FBT_LINK_WEAK_NOTIFY 			= 0,
	FBT_LINK_STRONG_NOTIFY 		= 1,
	FBT_LINK_ONLINE_NOTIFY 		= 2,
	FBT_LINK_OFFLINE_NOTIFY 		= 3,
	FBT_LINK_STA_FOUND_NOTIFY 	= 4,	
} FBT_NOTIFY_E;

typedef struct _FBT_NOTIFY_ST {
	UCHAR FbtNotifyExtApMac[20];
	UINT32 FbtChannel;
	INT32 FbtRSSI;
	FBT_NOTIFY_E FbtNotifyType;
	UCHAR FbtNotifyMac[20];	
} FBT_NOTIFY_ST, PFBT_NOTIFY_ST;

void wext_send_event(PNET_DEV net_dev, const char *peer_mac_addr,
			      		const char *bssid, UINT32 channel, INT32 rssi, FBT_NOTIFY_E event_type);

#endif /* __ALL_NET_EVENT__ */