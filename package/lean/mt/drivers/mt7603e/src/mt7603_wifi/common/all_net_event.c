#ifdef ALL_NET_EVENT
#include "rt_config.h"

void wext_send_event(PNET_DEV net_dev, const char *peer_mac_addr,
			      		const char *bssid, UINT32 channel, INT32 rssi, FBT_NOTIFY_E event_type)
{
	FBT_NOTIFY_ST event_data;

	memcpy(&event_data.FbtNotifyExtApMac, bssid, 6);
	event_data.FbtChannel = channel;
	event_data.FbtRSSI = rssi;

	event_data.FbtNotifyType = event_type;
	memcpy(&event_data.FbtNotifyMac, peer_mac_addr, 6);

	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM, 
					IW_ALL_NET_EVENT, NULL, (PUCHAR)&event_data, sizeof(event_data));

}
#endif /* ALL_NET_EVENT */
