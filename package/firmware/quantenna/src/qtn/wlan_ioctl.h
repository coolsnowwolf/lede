/*
 * Copyright (c) 2014 Quantenna Communications, Inc.
 * All rights reserved.
 */

#ifndef __WLAN_IOCTL_H__
#define __WLAN_IOCTL_H__

enum ieee80211_wifi_mode {
	IEEE80211_WIFI_MODE_NONE = 0,
	IEEE80211_WIFI_MODE_A,
	IEEE80211_WIFI_MODE_B,
	IEEE80211_WIFI_MODE_G,
	IEEE80211_WIFI_MODE_NA,
	IEEE80211_WIFI_MODE_NG,
	IEEE80211_WIFI_MODE_AC,
	IEEE80211_WIFI_MODE_MAX,
};

#define WLAN_WIFI_MODES_STRINGS		{		\
	[IEEE80211_WIFI_MODE_NONE] = "-",		\
	[IEEE80211_WIFI_MODE_A] = "a",			\
	[IEEE80211_WIFI_MODE_B] = "b",			\
	[IEEE80211_WIFI_MODE_G] = "g",			\
	[IEEE80211_WIFI_MODE_NA] = "na",		\
	[IEEE80211_WIFI_MODE_NG] = "ng",		\
	[IEEE80211_WIFI_MODE_AC] = "ac",		\
}

#define IEEE80211_HTCAP_IE_LENGTH	28
#define IEEE80211_VHTCAP_IE_LENGTH	14

struct ieee8011req_sta_tput_caps {
	uint8_t	macaddr[ETH_ALEN];
	uint8_t	mode;
	uint8_t	htcap_ie[IEEE80211_HTCAP_IE_LENGTH];
	uint8_t	vhtcap_ie[IEEE80211_VHTCAP_IE_LENGTH];
};
#endif /* __WLAN_IOCTL_H__ */
